#include "LevelEditor.h"
#include <CommCtrl.h>
#include <windowsx.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <filesystem>

// Initialize common controls
#pragma comment(lib, "comctl32.lib")
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Window dimensions
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;


#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    MessageBoxW(NULL, L"Hello, world!", L"Title", MB_OK);
    return 0;
}




// Implementation of LevelObject
LevelObject::LevelObject(const std::string& name, ObjectType type)
    : name_(name), type_(type) {
    position_[0] = position_[1] = position_[2] = 0.0f;
    rotation_[0] = rotation_[1] = rotation_[2] = 0.0f;
    scale_[0] = scale_[1] = scale_[2] = 1.0f;
}

LevelObject::~LevelObject() {
}

void LevelObject::SetPosition(float x, float y, float z) {
    position_[0] = x;
    position_[1] = y;
    position_[2] = z;
}

void LevelObject::SetRotation(float x, float y, float z) {
    rotation_[0] = x;
    rotation_[1] = y;
    rotation_[2] = z;
}

void LevelObject::SetScale(float x, float y, float z) {
    scale_[0] = x;
    scale_[1] = y;
    scale_[2] = z;
}

void LevelObject::SetProperty(const std::string& key, const std::string& value) {
    properties_[key] = value;
}

std::string LevelObject::GetProperty(const std::string& key) {
    auto it = properties_.find(key);
    if (it != properties_.end()) {
        return it->second;
    }
    return "";
}

void LevelObject::Serialize(std::ofstream& file) {
    file << "OBJECT\n";
    file << "NAME=" << name_ << "\n";
    file << "TYPE=" << static_cast<int>(type_) << "\n";
    file << "POSITION=" << position_[0] << "," << position_[1] << "," << position_[2] << "\n";
    file << "ROTATION=" << rotation_[0] << "," << rotation_[1] << "," << rotation_[2] << "\n";
    file << "SCALE=" << scale_[0] << "," << scale_[1] << "," << scale_[2] << "\n";

    file << "PROPERTIES_COUNT=" << properties_.size() << "\n";
    for (const auto& [key, value] : properties_) {
        file << "PROPERTY=" << key << "," << value << "\n";
    }
    file << "END_OBJECT\n";
}

std::unique_ptr<LevelObject> LevelObject::Deserialize(std::ifstream& file) {
    std::string line;
    std::string name;
    ObjectType type = ObjectType::Mesh;
    std::map<std::string, std::string> properties;
    float position[3] = { 0 };
    float rotation[3] = { 0 };
    float scale[3] = { 1, 1, 1 };

    // Read object data until END_OBJECT
    while (std::getline(file, line) && line != "END_OBJECT") {
        std::istringstream iss(line);
        std::string key;

        if (std::getline(iss, key, '=')) {
            std::string value = line.substr(key.length() + 1);

            if (key == "NAME") {
                name = value;
            }
            else if (key == "TYPE") {
                type = static_cast<ObjectType>(std::stoi(value));
            }
            else if (key == "POSITION") {
                sscanf_s(value.c_str(), "%f,%f,%f", &position[0], &position[1], &position[2]);
            }
            else if (key == "ROTATION") {
                sscanf_s(value.c_str(), "%f,%f,%f", &rotation[0], &rotation[1], &rotation[2]);
            }
            else if (key == "SCALE") {
                sscanf_s(value.c_str(), "%f,%f,%f", &scale[0], &scale[1], &scale[2]);
            }
            else if (key == "PROPERTY") {
                size_t commaPos = value.find(",");
                if (commaPos != std::string::npos) {
                    std::string propKey = value.substr(0, commaPos);
                    std::string propValue = value.substr(commaPos + 1);
                    properties[propKey] = propValue;
                }
            }
        }
    }

    // Create and return object
    auto object = std::make_unique<LevelObject>(name, type);
    object->SetPosition(position[0], position[1], position[2]);
    object->SetRotation(rotation[0], rotation[1], rotation[2]);
    object->SetScale(scale[0], scale[1], scale[2]);

    for (const auto& [propKey, propValue] : properties) {
        object->SetProperty(propKey, propValue);
    }

    return object;
}
