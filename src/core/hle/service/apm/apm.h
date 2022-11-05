// SPDX-FileCopyrightText: Copyright 2018 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

namespace Core {
class System;
}

namespace Service::APM {

class Module final {
public:
    Module();
    ~Module();
};

/// Registers all AM services with the specified service manager.
void InstallInterfaces(Core::System& system);

} // namespace Service::APM
