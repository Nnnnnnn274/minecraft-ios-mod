"""
MCCL Offset Discovery — Frida Script for iOS
==============================================
Run on your jailbroken/sideloaded iOS device to find C++ function addresses
at runtime and write them to mccl_offsets.txt for the dylib to load.

Usage:
    # Install Frida on device (via Cydia/Sileo or frida-server)
    # On Windows:
    pip install frida-tools
    # Connect iPhone via USB, then:
    frida -U -f com.mojang.minecraftpe -l find_offsets.py --no-pause

Outputs: /var/mobile/Documents/mccl_offsets.txt
"""

import frida
import sys
import json

OUTPUT_PATH = "/var/mobile/Documents/mccl_offsets.txt"

def write_offsets(offsets_dict):
    """Write offsets in format: name 0xADDRESS"""
    lines = []
    for name, addr in sorted(offsets_dict.items()):
        lines.append(f"{name} 0x{addr:X}")
    with open(OUTPUT_PATH, "w") as f:
        f.write("\n".join(lines) + "\n")
    print(f"[+] Wrote {len(offsets_dict)} offsets to {OUTPUT_PATH}")

def on_message(message, data):
    if message["type"] == "send":
        payload = message["payload"]
        if isinstance(payload, dict) and "offsets" in payload:
            write_offsets(payload["offsets"])
            sys.exit(0)

def make_script():
    return """
    const OFFSET_NAMES = {
        // Main game loop
        "Minecraft_update": ["Minecraft::update", "Minecraft::tick", "Minecraft::clientTick", "Minecraft::serverTick"],
        
        // Network handlers  
        "ServerNetworkHandler_tick": ["ServerNetworkHandler::tick", "ServerNetworkHandler::tickServer", "ServerNetworkHandler::processPackets"],
        "NetworkSystem_runEvents": ["NetworkSystem::runEvents", "NetworkSystem::tick", "NetworkSystem::processEvents"],
        
        // Level/world
        "Level_tick": ["Level::tick", "Level::serverTick", "Level::clientTick"],
        
        // Game mode
        "GameMode_tick": ["GameMode::tick", "GameMode::serverTick"],
        
        // Player
        "ServerPlayer_tick": ["ServerPlayer::tick", "Player::tick"],
        
        // World generation
        "WorldGen_generate": ["WorldGen::generate", "ChunkSource::generate"],
        
        // Commands
        "CommandManager_execute": ["CommandManager::execute", "CommandManager::executeCommand"],
        
        // Entity
        "Entity_tick": ["Entity::tick", "Actor::tick", "Mob::tick"],
        
        // Packet handling
        "PacketHandler_handle": ["PacketHandler::handle", "NetworkIdentifier::handlePacket"],
    };

    const MINECRAFT_MODULE = "minecraftpe";
    const DUMP_EVERYTHING = false;  // Set true to dump all exports

    function resolve(name, candidates) {
        for (let c of candidates) {
            const sym = Module.findExportByName(MINECRAFT_MODULE, c);
            if (sym) {
                console.log("[+] " + name + " -> " + sym + " (" + c + ")");
                return sym;
            }
        }
        return null;
    }

    function scanForPattern(pattern, description) {
        try {
            const matches = Memory.scanSync(Process.getModuleByName(MINECRAFT_MODULE).base, 
                                          Process.getModuleByName(MINECRAFT_MODULE).size,
                                          pattern);
            if (matches.length > 0) {
                console.log("[PATTERN] " + description + ": " + matches[0].address);
                return matches[0].address;
            }
        } catch(e) {}
        return null;
    }

    function findServerNetworkHandler() {
        // Try exports first
        let addr = resolve("ServerNetworkHandler_tick", OFFSET_NAMES.ServerNetworkHandler_tick);
        if (addr) return { "ServerNetworkHandler_tick": addr };
        
        // Fallback: scan for common arm64 patterns
        // ServerNetworkHandler::tick often has a specific prologue
        // STP X29, X30, [SP, #-0x40]! ; SUB SP, SP, #0x30 ; ...
        const pattern1 = "a9 bf 7f f0 f0 03 1f aa"; // STP X29,X30,[SP,#-0x40]! ; MOV X29,SP
        const pattern2 = "a9 bf 7f f0 f4 4f 01 a9"; // STP X20,X19,[SP,#-0x30]! ; STP X29,X30,[SP,#0x10]
        
        let matches = Memory.scanSync(Process.getModuleByName(MINECRAFT_MODULE).base,
                                      Process.getModuleByName(MINECRAFT_MODULE).size,
                                      pattern1);
        if (matches.length > 0) {
            console.log("[PATTERN] Found ServerNetworkHandler::tick candidate @ " + matches[0].address);
            return { "ServerNetworkHandler_tick": matches[0].address };
        }
        
        // Alternative: look for the string reference
        // Search for "ServerNetworkHandler::tick" string and find code that references it
        const strPtr = Module.findExportByName(MINECRAFT_MODULE, "ServerNetworkHandler::tick");
        if (strPtr) {
            // Find code that loads this string address (ADRP+ADD)
            const page = strPtr & ~0xFFF;
            const offset = strPtr & 0xFFF;
            // We'd need to scan code for ADRP+ADD targeting this - complex in Frida
        }
        
        return null;
    }

    function findMinecraftUpdate() {
        let addr = resolve("Minecraft_update", OFFSET_NAMES.Minecraft_update);
        if (addr) return { "Minecraft_update": addr };
        
        // Pattern: Minecraft::update often calls Level::tick
        const pattern = "94 00 00 00 94 00 00 00 94 00 00 00"; // Three consecutive BL calls
        // Too generic...
        
        return null;
    }

    function findNetworkSystemRunEvents() {
        let addr = resolve("NetworkSystem_runEvents", OFFSET_NAMES.NetworkSystem_runEvents);
        if (addr) return { "NetworkSystem_runEvents": addr };
        
        return null;
    }

    // --- MAIN ---
    console.log("[MCCL] Starting offset discovery...");
    
    const offsets = {};
    
    // Try exports first
    for (const [name, candidates] of Object.entries(OFFSET_NAMES)) {
        const addr = resolve(name, candidates);
        if (addr) offsets[name] = addr;
    }
    
    // Try pattern scanning for ServerNetworkHandler::tick
    if (!offsets.ServerNetworkHandler_tick) {
        const sn = findServerNetworkHandler();
        if (sn) Object.assign(offsets, sn);
    }
    
    // Try pattern scanning for Minecraft::update
    if (!offsets.Minecraft_update) {
        const mu = findMinecraftUpdate();
        if (mu) Object.assign(offsets, mu);
    }
    
    // Try pattern scanning for NetworkSystem
    if (!offsets.NetworkSystem_runEvents) {
        const ne = findNetworkSystemRunEvents();
        if (ne) Object.assign(offsets, ne);
    }
    
    // Also try to hook known classes via Objective-C if available
    if (ObjC.available) {
        try {
            const snClass = ObjC.classes.ServerNetworkHandler;
            if (snClass) {
                const methods = snClass.$methods;
                methods.forEach(m => {
                    if (m.includes("tick")) {
                        const impl = snClass[m].implementation;
                        if (impl) {
                            console.log("[OBJC] ServerNetworkHandler." + m + " @ " + impl);
                            offsets["ServerNetworkHandler_" + m] = impl;
                        }
                    }
                });
            }
        } catch(e) {}
    }
    
    // Dump all known addresses
    console.log("\n=== DISCOVERED OFFSETS ===");
    for (const [k, v] of Object.entries(offsets)) {
        console.log("  " + k + " = " + v);
    }
    
    // Send back to host
    send({ offsets: offsets });
    """ + "\n"

def main():
    print("[MCCL] Attaching to Minecraft...")
    try:
        device = frida.get_usb_device()
        pid = device.spawn(["com.mojang.minecraftpe"])
        session = device.attach(pid)
        device.resume(pid)
    except:
        print("[-] Could not spawn/attach. Is device connected and app installed?")
        return

    script = session.create_script(make_script())
    script.on("message", on_message)
    script.load()
    
    print("[MCCL] Running... (waiting for offsets)")
    sys.stdin.read()

if __name__ == "__main__":
    main()