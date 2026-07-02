#pragma once
#include <string>

namespace APILevel {
    enum Level { NONE, PAPERWEIGHT, HARDWEIGHT };
    Level getSelected();
}

void showAPISelector();
