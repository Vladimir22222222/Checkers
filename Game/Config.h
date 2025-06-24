#pragma once
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include "../Models/Project_path.h"

class Config
{
  public:
    Config()
    {
        reload();
    }
    /**
  * @brief ������������� ���������������� ������ �� JSON-�����
  *
  * ����� ��������� ��������� ��������:
  * 1. ��������� ���� �������� "settings.json" �� ���������� �������
  * 2. ������ ���������� ����� � ������ JSON
  * 3. ��������� �������� �����
  *
  * @note ���� ���� �� ���������� ��� �������� ������������ JSON,
  *       ����� ��������� ���������� �� ���������� nlohmann/json
  */

    void reload()
    {
        std::ifstream fin(project_path + "settings.json");
        fin >> config;
        fin.close();
    }
    /**
 * @brief �������� ������� � ���������� ������������
 *
 * @param setting_dir �������� ������� ������������ ������ ������� �����������
 * @param setting_name �������� ��������� ������ ������� �����������
 * @return json& ������ �� ����������� �������� �� ������������
 * @note ���� ������������� �������� �� ����������, ����� ���������
 *       ����������� ������ json, ������� ����� ��������� ������� is_null()
 */

    auto operator()(const string &setting_dir, const string &setting_name) const
    {
        return config[setting_dir][setting_name];
    }

  private:
    json config; // ������ ��� �������� ������������ � ������� JSON
};
