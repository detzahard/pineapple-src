// SPDX-FileCopyrightText: Copyright 2020 yuzu Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#include <forward_list>
#include <memory>
#include <typeinfo>
#include <QComboBox>
#include "common/common_types.h"
#include "common/settings.h"
#include "common/settings_enums.h"
#include "configuration/shared_widget.h"
#include "core/core.h"
#include "ui_configure_cpu.h"
#include "yuzu/configuration/configuration_shared.h"
#include "yuzu/configuration/configure_cpu.h"

ConfigureCpu::ConfigureCpu(const Core::System& system_,
                           std::shared_ptr<std::forward_list<ConfigurationShared::Tab*>> group_,
                           const ConfigurationShared::Builder& builder, QWidget* parent)
    : Tab(group_, parent), ui{std::make_unique<Ui::ConfigureCpu>()}, system{system_},
      combobox_translations(builder.ComboboxTranslations()) {
    ui->setupUi(this);

    Setup(builder);

    SetConfiguration();

    connect(accuracy_combobox, qOverload<int>(&QComboBox::currentIndexChanged), this,
            &ConfigureCpu::UpdateGroup);
}

ConfigureCpu::~ConfigureCpu() = default;

void ConfigureCpu::SetConfiguration() {}
void ConfigureCpu::Setup(const ConfigurationShared::Builder& builder) {
    auto* accuracy_layout = ui->widget_accuracy->layout();
    auto* unsafe_layout = ui->unsafe_widget->layout();
    std::map<std::string, QWidget*> unsafe_hold{};

    std::forward_list<Settings::BasicSetting*> settings;
    const auto push = [&](Settings::Category category) {
        for (const auto setting : Settings::values.linkage.by_category[category]) {
            settings.push_front(setting);
        }
    };

    push(Settings::Category::Cpu);
    push(Settings::Category::CpuUnsafe);

    for (const auto setting : settings) {
        auto* widget = builder.BuildWidget(setting, apply_funcs);

        if (widget == nullptr) {
            continue;
        }
        if (!widget->Valid()) {
            delete widget;
            continue;
        }

        if (setting->Id() == Settings::values.cpu_accuracy.Id()) {
            // Keep track of cpu_accuracy combobox to display/hide the unsafe settings
            accuracy_layout->addWidget(widget);
            accuracy_combobox = widget->combobox;
        } else {
            // Presently, all other settings here are unsafe checkboxes
            unsafe_hold.insert({setting->GetLabel(), widget});
        }
    }

    for (const auto& [label, widget] : unsafe_hold) {
        unsafe_layout->addWidget(widget);
    }

    UpdateGroup(accuracy_combobox->currentIndex());
}

void ConfigureCpu::UpdateGroup(int index) {
    const auto accuracy = static_cast<Settings::CpuAccuracy>(
        combobox_translations.at(Settings::EnumMetadata<Settings::CpuAccuracy>::Index())[index]
            .first);
    ui->unsafe_group->setVisible(accuracy == Settings::CpuAccuracy::Unsafe);
}

void ConfigureCpu::ApplyConfiguration() {
    const bool is_powered_on = system.IsPoweredOn();
    for (const auto& apply_func : apply_funcs) {
        apply_func(is_powered_on);
    }
}

void ConfigureCpu::changeEvent(QEvent* event) {
    if (event->type() == QEvent::LanguageChange) {
        RetranslateUI();
    }

    QWidget::changeEvent(event);
}

void ConfigureCpu::RetranslateUI() {
    ui->retranslateUi(this);
}
