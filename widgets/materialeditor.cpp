#include "materialeditor.h"
#include "ui_materialeditor.h"
#include "temperaturevaluedialog.h"
#include "toastdlg.h"
#include <QDir>
#include <QTextStream>
#include <QFile>
#include <QDebug>
#include <utility> // for std::pair

MaterialEditor::MaterialEditor(QWidget *parent)
    : FramelessBaseDialog(parent)
    , ui(new Ui::MaterialEditor)
{
    ui->setupUi(this);
    if (ui->mainContainer) {
        this->contentLayout()->addWidget(ui->mainContainer);
        ui->mainContainer->setVisible(true);
    }

    setWindowTitleText("Material Editor");
    this->contentLayout()->setContentsMargins(5, 5, 5, 5);
    this->resize(900, 600); // 稍微调大窗口以容纳更多Tab

    // --- 初始化下拉框和堆叠窗口 ---
    // 确保你的 comboBox_ModelType 已经在 UI 里添加了这两个 Item，或者在这里添加：
    if(ui->comboBox_ModelType->count() == 0) {
        ui->comboBox_ModelType->addItem("Elastic-Plastic");
        ui->comboBox_ModelType->addItem("Johnson-Cook (JC)");
    }

    // 连接信号：切换模型时切换界面
    connect(ui->comboBox_ModelType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MaterialEditor::on_comboBox_ModelType_currentIndexChanged);

    // 初始化显示第一页 (Elastic-Plastic)
    ui->stackedWidget_Constitutive->setCurrentIndex(0);

    // --- 设置所有 Global/Limits/Gas 参数的初始显示值 ---
    auto safeSetText = [](QLineEdit* edit, const QString& val){
        if(edit) edit->setText(val);
    };

    // Constants
    safeSetText(ui->lineEdit_Molar_mass, Molar_mass);
    safeSetText(ui->lineEdit_universal_gas_constant, universal_gas_constant);
    safeSetText(ui->lineEdit_Stefan_Boltzmann_constant, Stefan_Boltzmann_constant);
    safeSetText(ui->lineEdit_convection_coefficient, convection_coefficient);
    safeSetText(ui->lineEdit_radiative_emissivity, radiative_emissivity);
    safeSetText(ui->lineEdit_absorption, absorption);
    safeSetText(ui->lineEdit_reference_temperature, reference_temperature);
    safeSetText(ui->lineEdit_reference_pressure, reference_pressure);
    safeSetText(ui->lineEdit_latent_heat_vaporization, latent_heat_vaporization);

    // Limits (新增)
    safeSetText(ui->lineEdit_max_temperature, max_temperature);
    safeSetText(ui->lineEdit_min_temperature, min_temperature);
    safeSetText(ui->lineEdit_max_vapor_temperature, max_vapor_temperature);
    safeSetText(ui->lineEdit_max_vapor_heat_temperature, max_vapor_heat_temperature);
    safeSetText(ui->lineEdit_max_latent_iteration, max_latent_iteration);
    safeSetText(ui->lineEdit_latent_iteration_tolerance, latent_iteration_tolerance);

    // Gas (新增)
    safeSetText(ui->lineEdit_gas_density, gas_density);
    safeSetText(ui->lineEdit_gas_viscosity, gas_viscosity);
    safeSetText(ui->lineEdit_gas_thermal_conductivity, gas_thermal_conductivity);
    safeSetText(ui->lineEdit_gas_specific_heat, gas_specific_heat);
}


MaterialEditor::~MaterialEditor()
{
    delete ui;
}

void MaterialEditor::setTargetNodeName(const QString &name)
{
    m_currentEditingNode = name;
    if (name.isEmpty()) {
        setWindowTitleText("Material Editor");
    } else {
        setWindowTitleText("Edit Material: " + name);
    }
}

// --------------------------------------------------------------------------
//                              核心逻辑槽函数
// --------------------------------------------------------------------------

// 切换本构模型：自动隐藏/显示相关 LineEdit (通过 StackedWidget)
void MaterialEditor::on_comboBox_ModelType_currentIndexChanged(int index)
{
    // index 0 -> Elastic Plastic Page
    // index 1 -> Johnson Cook Page
    if(ui->stackedWidget_Constitutive) {
        ui->stackedWidget_Constitutive->setCurrentIndex(index);
    }
}


void MaterialEditor::on_pushButton_cancel_clicked()
{
    close();
}

// --------------------------------------------------------------------------
//                              Solid (力学) 部分
// --------------------------------------------------------------------------

// 这里的 Save 按钮处理 Solid 页面的所有数据，包括根据当前模式读取不同控件
void MaterialEditor::updateSolidData()
{
    // 1. 获取通用 Solid 参数
    solid_density = ui->lineEdit_solid_density->text().trimmed();
    poisson_ratio = ui->lineEdit_solid_poisson_ratio->text().trimmed();

    // 解析 "温度/值" 的 lambda
    auto parseTempValue = [](const QString& labelStr) -> std::pair<QString, QString> {
        QString s = labelStr.trimmed();
        if (s.isEmpty()) return {QString(), QString()};
        QStringList parts = s.split("/", QString::SkipEmptyParts);
        if (parts.size() >= 2) return {parts[0].trimmed(), parts[1].trimmed()};
        return {QString(), QString()};
    };

    std::tie(youngs_modulus_tem, youngs_modulus_val) = parseTempValue(ui->label_youngs_modulus->text());

    // 2. 根据当前选中的模型获取特定参数
    int currentModel = ui->comboBox_ModelType->currentIndex();

    if (currentModel == ElasticPlastic) {
        // --- 弹塑性模式 ---
        std::tie(yield_stress_tem, yield_stress_val) = parseTempValue(ui->label_yield_stress->text());
        std::tie(plastic_modulus_tem, plastic_modulus_val) = parseTempValue(ui->label_plastic_modulus->text());

        if (youngs_modulus_tem.isEmpty()) Toast::instance().show(Toast::TINFO, "Youngs Modulus incomplete", this);
        // 在这里可以加更多弹塑性的校验
    }
    else if (currentModel == JohnsonCook) {
        // --- JC 模式 ---
        jc_A = ui->lineEdit_JC_A->text().trimmed();
        jc_B = ui->lineEdit_JC_B->text().trimmed();
        jc_C = ui->lineEdit_JC_C->text().trimmed();
        jc_n = ui->lineEdit_JC_n->text().trimmed();
        jc_m = ui->lineEdit_JC_m->text().trimmed();

        if (jc_A.isEmpty() || jc_B.isEmpty()) {
            Toast::instance().show(Toast::TINFO, "JC Parameters incomplete", this);
        }
    }

    // Toast::instance().show(Toast::TINFO, "Solid Settings Saved Temporarily", this);
}

// 弹窗辅助按钮
void MaterialEditor::on_pushButton_youngs_modulus_clicked() {
    TemperatureValueDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) ui->label_youngs_modulus->setText(dialog.getOutput());
}
void MaterialEditor::on_pushButton_yield_stress_clicked() {
    TemperatureValueDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) ui->label_yield_stress->setText(dialog.getOutput());
}
void MaterialEditor::on_plastic_modulus_clicked() {
    TemperatureValueDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) ui->label_plastic_modulus->setText(dialog.getOutput());
}

// --------------------------------------------------------------------------
//                              Liquid (流体) 部分
// --------------------------------------------------------------------------

void MaterialEditor::updateLiquidData()
{
    surface_tension_coefficient = ui->lineEdit_surface_tension_coefficient->text().trimmed();
    Marangoni_coefficient = ui->lineEdit_Marangoni_coefficient->text().trimmed();
    permeability_coefficient = ui->lineEdit_permeability_coefficient->text().trimmed();

    auto parseTempValue = [](const QString& labelStr) -> std::pair<QString, QString> {
        QString s = labelStr.trimmed();
        if (s.isEmpty()) return {QString(), QString()};
        QStringList parts = s.split("/", QString::SkipEmptyParts);
        if (parts.size() >= 2) return {parts[0].trimmed(), parts[1].trimmed()};
        return {QString(), QString()};
    };

    std::tie(fluid_density_tem, fluid_density_val) = parseTempValue(ui->label_fluid_density->text());
    std::tie(dynamic_viscosity_tem, dynamic_viscosity_val) = parseTempValue(ui->label_dynamic_viscosity->text());
    std::tie(thermal_conductivity_tem, thermal_conductivity_val) = parseTempValue(ui->label_thermal_conductivity->text());
    std::tie(specific_heat_tem, specific_heat_val) = parseTempValue(ui->label_specific_heat->text());

    if (fluid_density_tem.isEmpty()) Toast::instance().show(Toast::TINFO, "Fluid density incomplete", this);
    // ... 其他校验
    // Toast::instance().show(Toast::TINFO, "Liquid Settings Saved Temporarily", this);
}

void MaterialEditor::on_pushButton_fluid_density_clicked() {
    TemperatureValueDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) ui->label_fluid_density->setText(dialog.getOutput());
}
void MaterialEditor::on_pushButton_dynamic_viscosity_clicked() {
    TemperatureValueDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) ui->label_dynamic_viscosity->setText(dialog.getOutput());
}
void MaterialEditor::on_pushbutton_thermal_conductivity_clicked() {
    TemperatureValueDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) ui->label_thermal_conductivity->setText(dialog.getOutput());
}
void MaterialEditor::on_pushButton_specific_heat_clicked() {
    TemperatureValueDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) ui->label_specific_heat->setText(dialog.getOutput());
}

// --------------------------------------------------------------------------
//                              Thermology (热学) 部分
// --------------------------------------------------------------------------

void MaterialEditor::updateThermologyData()
{
    liquidus_temperature = ui->liquidus_temperature->text().trimmed();
    solidus_temperature = ui->solidus_temperature->text().trimmed();
    evaporation_temperature = ui->evaporation_temperature->text().trimmed();
    latent_heat_melting = ui->latent_heat_melting->text().trimmed();
    vapor_pressure_coefficient = ui->vapor_pressure_coefficient->text().trimmed();
    vapor_heat_loss_coefficient = ui->vapor_heat_loss_coefficient->text().trimmed();
    thermal_expansion = ui->thermal_expansion->text().trimmed();
    // Toast::instance().show(Toast::TINFO, "Thermology Settings Saved Temporarily", this);
}

// --------------------------------------------------------------------------
//                          Global (全局/物理常数) 部分 [新增]
// --------------------------------------------------------------------------

void MaterialEditor::updateGlobalData()
{
    auto readText = [](QLineEdit* edit, QString& target){
        if(edit && !edit->text().isEmpty()) target = edit->text().trimmed();
    };

    // Constants
    readText(ui->lineEdit_Molar_mass, Molar_mass);
    readText(ui->lineEdit_universal_gas_constant, universal_gas_constant);
    readText(ui->lineEdit_Stefan_Boltzmann_constant, Stefan_Boltzmann_constant);
    readText(ui->lineEdit_convection_coefficient, convection_coefficient);
    readText(ui->lineEdit_radiative_emissivity, radiative_emissivity);
    readText(ui->lineEdit_absorption, absorption);
    readText(ui->lineEdit_reference_temperature, reference_temperature);
    readText(ui->lineEdit_reference_pressure, reference_pressure);
    readText(ui->lineEdit_latent_heat_vaporization, latent_heat_vaporization);

    // Limits
    readText(ui->lineEdit_max_temperature, max_temperature);
    readText(ui->lineEdit_min_temperature, min_temperature);
    readText(ui->lineEdit_max_vapor_temperature, max_vapor_temperature);
    readText(ui->lineEdit_max_vapor_heat_temperature, max_vapor_heat_temperature);
    readText(ui->lineEdit_max_latent_iteration, max_latent_iteration);
    readText(ui->lineEdit_latent_iteration_tolerance, latent_iteration_tolerance);

    // Gas
    readText(ui->lineEdit_gas_density, gas_density);
    readText(ui->lineEdit_gas_viscosity, gas_viscosity);
    readText(ui->lineEdit_gas_thermal_conductivity, gas_thermal_conductivity);
    readText(ui->lineEdit_gas_specific_heat, gas_specific_heat);

    // Toast::instance().show(Toast::TINFO, "Global Settings Saved Temporarily", this);
}

// --------------------------------------------------------------------------
//                          YAML 生成与最终保存
// --------------------------------------------------------------------------

QString MaterialEditor::getYamlSection(const QString &refTempOverride) const
{
    QString yaml;
    QTextStream out(&yaml);

    out << "material:\n";

    // 1. 本构模型
    if (ui->comboBox_ModelType->currentIndex() == JohnsonCook) {
        out << "    # Johnson-Cook Model\n";
        out << "    A: " << jc_A << "\n";
        out << "    B: " << jc_B << "\n";
        out << "    C: " << jc_C << "\n";
        out << "    n: " << jc_n << "\n";
        out << "    m: " << jc_m << "\n";
    } else {
        out << "    # Elastic-Plastic Model\n";
    }

    // 2. Solid Common
    if (!youngs_modulus_tem.isEmpty() && !youngs_modulus_val.isEmpty()) {
        out << "    youngs_modulus:\n";
        out << "        temperature: " << youngs_modulus_tem << "\n";
        out << "        value: " << youngs_modulus_val << "\n";
    }

    out << "    poisson_ratio: " << poisson_ratio << "\n";
    out << "    solid_density: " << solid_density << "\n";

    // 3. Solid Exclusive (Elastic-Plastic only)
    if (ui->comboBox_ModelType->currentIndex() == ElasticPlastic) {
        if (!yield_stress_tem.isEmpty() && !yield_stress_val.isEmpty()) {
            out << "    yield_stress:\n";
            out << "        temperature: " << yield_stress_tem << "\n";
            out << "        value: " << yield_stress_val << "\n";
        }
        if (!plastic_modulus_tem.isEmpty() && !plastic_modulus_val.isEmpty()) {
            out << "    plastic_modulus:\n";
            out << "        temperature: " << plastic_modulus_tem << "\n";
            out << "        value: " << plastic_modulus_val << "\n";
        }
    }

    // 4. Liquid/Viscosity
    if (!fluid_density_tem.isEmpty() && !fluid_density_val.isEmpty()) {
        out << "    fluid_density:\n";
        out << "        temperature: " << fluid_density_tem << "\n";
        out << "        value: " << fluid_density_val << "\n";
    }

    if (!dynamic_viscosity_tem.isEmpty() && !dynamic_viscosity_val.isEmpty()) {
        out << "    dynamic_viscosity:\n";
        out << "        temperature: " << dynamic_viscosity_tem << "\n";
        out << "        value: " << dynamic_viscosity_val << "\n";
    }

    if (!thermal_conductivity_tem.isEmpty() && !thermal_conductivity_val.isEmpty()) {
        out << "    thermal_conductivity:\n";
        out << "        temperature: " << thermal_conductivity_tem << "\n";
        out << "        value: " << thermal_conductivity_val << "\n";
    }

    if (!specific_heat_tem.isEmpty() && !specific_heat_val.isEmpty()) {
        out << "    specific_heat:\n";
        out << "        temperature: " << specific_heat_tem << "\n";
        out << "        value: " << specific_heat_val << "\n";
    }

    // 5. Phase Change & Thermal
    out << "    liquidus_temperature: " << liquidus_temperature << "\n";
    out << "    solidus_temperature: " << solidus_temperature << "\n";
    out << "    evaporation_temperature: " << evaporation_temperature << "\n";
    out << "    latent_heat_melting: " << latent_heat_melting << "\n";
    out << "    latent_heat_vaporization: " << latent_heat_vaporization << "\n";

    out << "    surface_tension_coefficient: " << surface_tension_coefficient << "\n";
    out << "    Marangoni_coefficient: " << Marangoni_coefficient << "\n";

    out << "    Molar_mass: " << Molar_mass << "\n";
    out << "    universal_gas_constant: " << universal_gas_constant << "\n";
    out << "    Stefan_Boltzmann_constant: " << Stefan_Boltzmann_constant << "\n";

    out << "    vapor_pressure_coefficient: " << vapor_pressure_coefficient << "\n";
    out << "    vapor_heat_loss_coefficient: " << vapor_heat_loss_coefficient << "\n";
    out << "    permeability_coefficient: " << permeability_coefficient << "\n";
    out << "    thermal_expansion: " << thermal_expansion << "\n";

    out << "    convection_coefficient: " << convection_coefficient << "\n";
    out << "    radiative_emissivity: " << radiative_emissivity << "\n";
    out << "    absorption: " << absorption << "\n";
    QString refTemp = refTempOverride.isEmpty() ? reference_temperature : refTempOverride;
    out << "    reference_temperature: " << refTemp << "\n";
    out << "    reference_pressure: " << reference_pressure << "\n";

    // 6. Limits & Gas (新增写入)
    out << "    max_temperature: " << max_temperature << "\n";
    out << "    min_temperature: " << min_temperature << "\n";
    out << "    max_vapor_temperature: " << max_vapor_temperature << "\n";
    out << "    max_vapor_heat_temperature: " << max_vapor_heat_temperature << "\n";
    out << "    max_latent_iteration: " << max_latent_iteration << "\n";
    out << "    latent_iteration_tolerance: " << latent_iteration_tolerance << "\n";

    out << "    gas_density: " << gas_density << "\n";
    out << "    gas_viscosity: " << gas_viscosity << "\n";
    out << "    gas_thermal_conductivity: " << gas_thermal_conductivity << "\n";
    out << "    gas_specific_heat: " << gas_specific_heat << "\n";

    return yaml;
}

void MaterialEditor::on_pushButton_OK_clicked()
{
    // 调用新的辅助函数，更新所有数据
    updateSolidData();
    updateLiquidData();
    updateThermologyData();
    updateGlobalData();

    Toast::instance().show(Toast::TINFO, "Material Saved Successfully", this);

    emit sigAddMaterial(ui->comboBox_Material->currentText());
    accept();
}
