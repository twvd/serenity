/*
 * Copyright (c) 2020, Luke Wilde <lukew@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "RegistersModel.h"

namespace HackStudio {

RegistersModel::RegistersModel(const PtraceRegisters& regs)
    : m_raw_registers(regs)
{
    m_registers.append({ "eax", regs.eax });
    m_registers.append({ "ebx", regs.ebx });
    m_registers.append({ "ecx", regs.ecx });
    m_registers.append({ "edx", regs.edx });
    m_registers.append({ "esp", regs.esp });
    m_registers.append({ "ebp", regs.ebp });
    m_registers.append({ "esi", regs.esi });
    m_registers.append({ "edi", regs.edi });
    m_registers.append({ "eip", regs.eip });
    m_registers.append({ "eflags", regs.eflags });
    m_registers.append({ "cs", regs.cs });
    m_registers.append({ "ss", regs.ss });
    m_registers.append({ "ds", regs.ds });
    m_registers.append({ "es", regs.es });
    m_registers.append({ "fs", regs.fs });
    m_registers.append({ "gs", regs.gs });
}

RegistersModel::RegistersModel(const PtraceRegisters& current_regs, const PtraceRegisters& previous_regs)
    : m_raw_registers(current_regs)
{
    m_registers.append({ "eax", current_regs.eax, current_regs.eax != previous_regs.eax });
    m_registers.append({ "ebx", current_regs.ebx, current_regs.ebx != previous_regs.ebx });
    m_registers.append({ "ecx", current_regs.ecx, current_regs.ecx != previous_regs.ecx });
    m_registers.append({ "edx", current_regs.edx, current_regs.edx != previous_regs.edx });
    m_registers.append({ "esp", current_regs.esp, current_regs.esp != previous_regs.esp });
    m_registers.append({ "ebp", current_regs.ebp, current_regs.ebp != previous_regs.ebp });
    m_registers.append({ "esi", current_regs.esi, current_regs.esi != previous_regs.esi });
    m_registers.append({ "edi", current_regs.edi, current_regs.edi != previous_regs.edi });
    m_registers.append({ "eip", current_regs.eip, current_regs.eip != previous_regs.eip });
    m_registers.append({ "eflags", current_regs.eflags, current_regs.eflags != previous_regs.eflags });
    m_registers.append({ "cs", current_regs.cs, current_regs.cs != previous_regs.cs });
    m_registers.append({ "ss", current_regs.ss, current_regs.ss != previous_regs.ss });
    m_registers.append({ "ds", current_regs.ds, current_regs.ds != previous_regs.ds });
    m_registers.append({ "es", current_regs.es, current_regs.es != previous_regs.es });
    m_registers.append({ "fs", current_regs.fs, current_regs.ds != previous_regs.fs });
    m_registers.append({ "gs", current_regs.gs, current_regs.gs != previous_regs.gs });
}

RegistersModel::~RegistersModel()
{
}

int RegistersModel::row_count(const GUI::ModelIndex&) const
{
    return m_registers.size();
}

String RegistersModel::column_name(int column) const
{
    switch (column) {
    case Column::Register:
        return "Register";
    case Column::Value:
        return "Value";
    default:
        VERIFY_NOT_REACHED();
        return {};
    }
}

GUI::Variant RegistersModel::data(const GUI::ModelIndex& index, GUI::ModelRole role) const
{
    auto& reg = m_registers[index.row()];

    if (role == GUI::ModelRole::ForegroundColor) {
        if (reg.changed)
            return Color(Color::Red);
        else
            return Color(Color::Black);
    }

    if (role == GUI::ModelRole::Display) {
        if (index.column() == Column::Register)
            return reg.name;
        if (index.column() == Column::Value)
            return String::formatted("{:p}", reg.value);
        return {};
    }
    return {};
}

void RegistersModel::update()
{
    did_update();
}

}
