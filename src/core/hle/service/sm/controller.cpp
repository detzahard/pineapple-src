// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "common/assert.h"
#include "common/logging/log.h"
#include "core/hle/ipc_helpers.h"
#include "core/hle/kernel/k_client_session.h"
#include "core/hle/kernel/k_server_session.h"
#include "core/hle/kernel/k_session.h"
#include "core/hle/service/sm/controller.h"

namespace Service::SM {

void Controller::ConvertCurrentObjectToDomain(Kernel::HLERequestContext& ctx) {
    ASSERT_MSG(ctx.Session()->IsSession(), "Session is already a domain");
    LOG_DEBUG(Service, "called, server_session={}", ctx.Session()->GetId());
    ctx.Session()->ConvertToDomain();

    IPC::ResponseBuilder rb{ctx, 3};
    rb.Push(RESULT_SUCCESS);
    rb.Push<u32>(1); // Converted sessions start with 1 request handler
}

void Controller::DuplicateSession(Kernel::HLERequestContext& ctx) {
    // TODO(bunnei): This is just creating a new handle to the same Session. I assume this is wrong
    // and that we probably want to actually make an entirely new Session, but we still need to
    // verify this on hardware.

    LOG_DEBUG(Service, "called");

    auto session = ctx.Session()->GetParent();

    // Open a reference to the session to simulate a new one being created.
    session->Open();
    session->GetClientSession().Open();
    session->GetServerSession().Open();

    IPC::ResponseBuilder rb{ctx, 2, 0, 1, IPC::ResponseBuilder::Flags::AlwaysMoveHandles};
    rb.Push(RESULT_SUCCESS);
    rb.PushMoveObjects(session->GetClientSession());
}

void Controller::DuplicateSessionEx(Kernel::HLERequestContext& ctx) {
    LOG_DEBUG(Service, "called");

    DuplicateSession(ctx);
}

void Controller::QueryPointerBufferSize(Kernel::HLERequestContext& ctx) {
    LOG_WARNING(Service, "(STUBBED) called");

    IPC::ResponseBuilder rb{ctx, 3};
    rb.Push(RESULT_SUCCESS);
    rb.Push<u16>(0x8000);
}

// https://switchbrew.org/wiki/IPC_Marshalling
Controller::Controller(Core::System& system_) : ServiceFramework{system_, "IpcController"} {
    static const FunctionInfo functions[] = {
        {0, &Controller::ConvertCurrentObjectToDomain, "ConvertCurrentObjectToDomain"},
        {1, nullptr, "CopyFromCurrentDomain"},
        {2, &Controller::DuplicateSession, "DuplicateSession"},
        {3, &Controller::QueryPointerBufferSize, "QueryPointerBufferSize"},
        {4, &Controller::DuplicateSessionEx, "DuplicateSessionEx"},
    };
    RegisterHandlers(functions);
}

Controller::~Controller() = default;

} // namespace Service::SM
