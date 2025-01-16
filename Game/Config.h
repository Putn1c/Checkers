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
        reload(); // В конструкторе происходит вызов функции reload(), которая загружает конфигурацию из файла settings.json
    }

    void reload()
    {
        std::ifstream fin(project_path + "settings.json"); // Открываем файл настроек (settings.json)
        fin >> config; // Читаем содержимое файла и записываем его в объект json 'config'
        fin.close(); // Закрываем файл после чтения
    }

    auto operator()(const string& setting_dir, const string& setting_name) const
    {
        // Оператор круглых скобок позволяет использовать экземпляр класса Config как функцию,
        // возвращая значение настройки из объекта конфигурации 'config' по предоставленным параметрам:
        // 'setting_dir' - направление настроек (например, категория), 
        // 'setting_name' - имя конкретной настройки.
        return config[setting_dir][setting_name];
    }

private:
    json config; // Объект для хранения конфигурации в формате JSON
};
