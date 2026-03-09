#ifndef MATERIALEDITOR_H
#define MATERIALEDITOR_H
#include "framelessbasedialog.h"

namespace Ui {
class MaterialEditor;
}

class MaterialEditor : public FramelessBaseDialog
{
    Q_OBJECT

public:
    explicit MaterialEditor(QWidget *parent = nullptr);
    ~MaterialEditor();

    // 定义本构模型枚举，方便代码可读性
    enum ModelType {
        ElasticPlastic = 0,
        JohnsonCook = 1
    };

private slots:
    // --- 核心逻辑槽 ---
    void on_comboBox_ModelType_currentIndexChanged(int index); // 新增：切换模型界面
    void on_pushButton_OK_clicked();     // 生成最终 YAML
    void on_pushButton_cancel_clicked();

    // --- 原有按钮槽 (Solid) ---
    void on_pushButton_youngs_modulus_clicked();
    void on_pushButton_yield_stress_clicked();   // 仅在 Elastic-Plastic 模式有效
    void on_plastic_modulus_clicked();           // 仅在 Elastic-Plastic 模式有效

    // --- 原有按钮槽 (Liquid/Thermology) ---
    void on_pushButton_fluid_density_clicked();
    void on_pushButton_dynamic_viscosity_clicked();
    void on_pushbutton_thermal_conductivity_clicked();
    void on_pushButton_specific_heat_clicked();

signals:
    void sigAddMaterial(QString name);

private:
    Ui::MaterialEditor *ui;

    // 辅助函数：生成 YAML 字符串
    QString generateYamlString();

    // --- 成员变量 (Data Fields) ---

    // 1. Solid - Common (通用)
    QString solid_density = "8179";
    QString poisson_ratio = "0.3";
    QString youngs_modulus_tem;
    QString youngs_modulus_val;

    // 2. Solid - Elastic-Plastic (弹塑性专用)
    QString yield_stress_tem;
    QString yield_stress_val;
    QString plastic_modulus_tem;
    QString plastic_modulus_val;

    // 3. Solid - Johnson-Cook (JC 专用) [新增]
    QString jc_A = "0.98e9";
    QString jc_B = "1.37e9";
    QString jc_C = "0.02";
    QString jc_n = "0.164";
    QString jc_m = "1.03";

    // 4. Liquid (流体)
    QString fluid_density_tem;
    QString fluid_density_val;
    QString dynamic_viscosity_tem;
    QString dynamic_viscosity_val;
    QString surface_tension_coefficient = "1.882";
    QString Marangoni_coefficient = "-1.1e-4";
    QString permeability_coefficient = "1e6";

    // 5. Thermology (热学)
    QString thermal_conductivity_tem;
    QString thermal_conductivity_val;
    QString specific_heat_tem;
    QString specific_heat_val;
    QString liquidus_temperature = "1609";
    QString solidus_temperature = "1533";
    QString evaporation_temperature = "3005";
    QString latent_heat_melting = "2.1e5";
    QString vapor_pressure_coefficient = "0.54";
    QString vapor_heat_loss_coefficient = "0.82"; // 注意：Input文件里它和vapor_heat_loss_coefficient有时候名字像，这里保留你的定义
    QString thermal_expansion = "15e-6";

    // 6. Global Constants (物理常数/环境) [新增]
    QString Molar_mass = "0.05975";
    QString universal_gas_constant = "8.314";
    QString Stefan_Boltzmann_constant = "5.67e-8";
    QString convection_coefficient = "5";
    QString radiative_emissivity = "0.4";
    QString absorption = "0.4";
    QString reference_temperature = "300";
    QString reference_pressure = "101000";
    QString latent_heat_vaporization = "6.31e6";

    // --- 7. Limits & Gas (新增缺失项) ---
    QString max_temperature = "3500";
    QString min_temperature = "0";
    QString max_vapor_temperature = "3010";
    QString max_vapor_heat_temperature = "3010";
    QString max_latent_iteration = "20";
    QString latent_iteration_tolerance = "1e-4";

    QString gas_density = "1";
    QString gas_viscosity = "1.48e-5";
    QString gas_thermal_conductivity = "0.02";
    QString gas_specific_heat = "1164";

private:
    // 内部辅助函数：从UI读取数据到变量
    void updateSolidData();
    void updateLiquidData();
    void updateThermologyData();
    void updateGlobalData();
};

#endif // MATERIALEDITOR_H
