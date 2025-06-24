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
  * @brief Перезагружает конфигурационные данные из JSON-файла
  *
  * Метод выполняет следующие действия:
  * 1. Открывает файл настроек "settings.json" из директории проекта
  * 2. Парсит содержимое файла в объект JSON
  * 3. Закрывает файловый поток
  *
  * @note Если файл не существует или содержит некорректный JSON,
  *       будет выброшено исключение из библиотеки nlohmann/json
  */

    void reload()
    {
        std::ifstream fin(project_path + "settings.json");
        fin >> config;
        fin.close();
    }
    /**
 * @brief Оператор доступа к настройкам конфигурации
 *
 * @param setting_dir Название раздела конфигурации первый уровень вложенности
 * @param setting_name Название параметра второй уровень вложенности
 * @return json& Ссылка на запрошенное значение из конфигурации
 * @note Если запрашиваемый параметр не существует, будет возвращён
 *       специальный объект json, который можно проверить методом is_null()
 */

    auto operator()(const string &setting_dir, const string &setting_name) const
    {
        return config[setting_dir][setting_name];
    }

  private:
    json config; // Объект для хранения конфигурации в формате JSON
};
