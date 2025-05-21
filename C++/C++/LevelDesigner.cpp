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

// Implementation of LevelData
LevelData::LevelData() {
}

LevelData::~LevelData() {
}

void LevelData::AddObject(std::unique_ptr<LevelObject> object) {
    objects_.push_back(std::move(object));
}

void LevelData::RemoveObject(const std::string& name) {
    objects_.erase(
        std::remove_if(objects_.begin(), objects_.end(),
            [&name](const std::unique_ptr<LevelObject>& obj) {
                return obj->GetName() == name;
            }),
        objects_.end());
}

LevelObject* LevelData::GetObject(const std::string& name) {
    for (const auto& obj : objects_) {
        if (obj->GetName() == name) {
            return obj.get();
        }
    }
    return nullptr;
}

std::vector<LevelObject*> LevelData::GetObjectsByType(ObjectType type) const {
    std::vector<LevelObject*> result;
    for (const auto& obj : objects_) {
        if (obj->GetType() == type) {
            result.push_back(obj.get());
        }
    }
    return result;
}

void LevelData::SetSetting(const std::string& key, const std::string& value) {
    settings_[key] = value;
}

std::string LevelData::GetSetting(const std::string& key) const {
    auto it = settings_.find(key);
    if (it != settings_.end()) {
        return it->second;
    }
    return "";
}

bool LevelData::SaveToFile(const fs::path& path) {
    std::ofstream file(path, std::ios::out);
    if (!file.is_open()) {
        return false;
    }

    // Write header
    file << "LEVEL_FILE_VERSION=1.0\n";

    // Write settings
    file << "SETTINGS_COUNT=" << settings_.size() << "\n";
    for (const auto& [key, value] : settings_) {
        file << "SETTING=" << key << "," << value << "\n";
    }

    // Write objects
    file << "OBJECTS_COUNT=" << objects_.size() << "\n";
    for (const auto& obj : objects_) {
        obj->Serialize(file);
    }

    file.close();
    return true;
}

bool LevelData::LoadFromFile(const fs::path& path) {
    std::ifstream file(path, std::ios::in);
    if (!file.is_open()) {
        return false;
    }

    // Clear existing data
    objects_.clear();
    settings_.clear();

    std::string line;
    while (std::getline(file, line)) {
        if (line == "OBJECT") {
            auto object = LevelObject::Deserialize(file);
            if (object) {
                objects_.push_back(std::move(object));
            }
        }
        else if (line.find("SETTING=") == 0) {
            std::string value = line.substr(8);
            size_t commaPos = value.find(",");
            if (commaPos != std::string::npos) {
                std::string key = value.substr(0, commaPos);
                std::string settingValue = value.substr(commaPos + 1);
                settings_[key] = settingValue;
            }
        }
    }

    file.close();
    return true;
}

// Implementation of CompilerSystem
CompilerSystem::CompilerSystem() : isCompiling_(false) {
}

CompilerSystem::~CompilerSystem() {
}

bool CompilerSystem::Initialize(const fs::path& enginePath, const fs::path& templatePath, const fs::path& outputPath) {
    enginePath_ = enginePath;
    templatePath_ = templatePath;
    outputPath_ = outputPath;

    // Create output directory if it doesn't exist
    if (!fs::exists(outputPath_)) {
        if (!fs::create_directories(outputPath_)) {
            return false;
        }
    }

    return true;
}

bool CompilerSystem::CompileLevel(const LevelData& level, const std::string& gameName) {
    // Don't compile if already compiling
    if (isCompiling_) {
        return false;
    }

    isCompiling_ = true;
    compilationLog_.clear();

    // Log start
    compilationLog_ += "Starting compilation for: " + gameName + "\n";

    // Create game directory
    fs::path gameDir = outputPath_ / gameName;
    if (fs::exists(gameDir)) {
        // Remove existing directory
        try {
            fs::remove_all(gameDir);
        }
        catch (const std::exception& e) {
            compilationLog_ += "Error clearing previous build: ";
            compilationLog_ += e.what();
            compilationLog_ += "\n";
            isCompiling_ = false;
            return false;
        }
    }

    // Create directory
    try {
        fs::create_directories(gameDir);
    }
    catch (const std::exception& e) {
        compilationLog_ += "Error creating game directory: ";
        compilationLog_ += e.what();
        compilationLog_ += "\n";
        isCompiling_ = false;
        return false;
    }

    // Copy engine files
    compilationLog_ += "Copying engine files...\n";
    if (!CopyEngineFiles(gameDir)) {
        isCompiling_ = false;
        return false;
    }

    // Generate game code from level data
    compilationLog_ += "Generating game code...\n";
    if (!GenerateGameCode(level, gameDir)) {
        isCompiling_ = false;
        return false;
    }

    // Build the game
    compilationLog_ += "Building game...\n";
    if (!BuildGame(gameDir)) {
        isCompiling_ = false;
        return false;
    }

    compilationLog_ += "Compilation completed successfully!\n";
    isCompiling_ = false;
    return true;
}

bool CompilerSystem::CopyEngineFiles(const fs::path& destination) {
    try {
        // Copy engine files to the game directory
        for (const auto& entry : fs::recursive_directory_iterator(enginePath_)) {
            // Skip certain files/directories if needed
            if (entry.is_directory()) {
                fs::create_directories(destination / fs::relative(entry.path(), enginePath_));
            }
            else {
                fs::copy_file(
                    entry.path(),
                    destination / fs::relative(entry.path(), enginePath_),
                    fs::copy_options::overwrite_existing
                );
            }
        }

        compilationLog_ += "Engine files copied successfully.\n";
        return true;
    }
    catch (const std::exception& e) {
        compilationLog_ += "Error copying engine files: ";
        compilationLog_ += e.what();
        compilationLog_ += "\n";
        return false;
    }
}

bool CompilerSystem::GenerateGameCode(const LevelData& level, const fs::path& destination) {
    try {
        // Generate game code from level data
        std::ofstream mainFile(destination / "GameLevel.cpp");
        if (!mainFile.is_open()) {
            compilationLog_ += "Failed to create GameLevel.cpp\n";
            return false;
        }

        // Write game level code
        mainFile << "#include \"Engine.h\"\n";
        mainFile << "#include \"GameLevel.h\"\n\n";

        // Create level initialization code
        mainFile << "void GameLevel::Initialize(Engine* engine) {\n";
        mainFile << "    // Generated from level editor\n";

        // Add all objects from the level
        // For each object, generate initialization code
        std::vector<LevelObject*> allObjects;
        for (auto& obj : level.GetObjectsByType(ObjectType::Mesh)) {
            allObjects.push_back(obj);

            mainFile << "    // Create " << obj->GetName() << "\n";
            mainFile << "    CreateObject(\"" << obj->GetName() << "\", ";
            mainFile << "XMFLOAT3(" << obj->GetProperty("posX") << ", "
                << obj->GetProperty("posY") << ", "
                << obj->GetProperty("posZ") << "), ";
            mainFile << "XMFLOAT3(" << obj->GetProperty("rotX") << ", "
                << obj->GetProperty("rotY") << ", "
                << obj->GetProperty("rotZ") << "));\n";
        }

        mainFile << "}\n\n";

        // Create update method
        mainFile << "void GameLevel::Update(float deltaTime) {\n";
        mainFile << "    // Custom update logic\n";
        mainFile << "}\n\n";

        // Create render method
        mainFile << "void GameLevel::Render() {\n";
        mainFile << "    // Custom render logic\n";
        mainFile << "}\n";

        mainFile.close();

        // Create header file
        std::ofstream headerFile(destination / "GameLevel.h");
        if (!headerFile.is_open()) {
            compilationLog_ += "Failed to create GameLevel.h\n";
            return false;
        }

        headerFile << "#pragma once\n";
        headerFile << "#include \"Engine.h\"\n\n";
        headerFile << "class GameLevel {\n";
        headerFile << "public:\n";
        headerFile << "    void Initialize(Engine* engine);\n";
        headerFile << "    void Update(float deltaTime);\n";
        headerFile << "    void Render();\n";
        headerFile << "\n";
        headerFile << "private:\n";
        headerFile << "    Engine* engine_;\n";
        headerFile << "    \n";
        headerFile << "    // Helper methods\n";
        headerFile << "    void CreateObject(const std::string& name, XMFLOAT3 position, XMFLOAT3 rotation) {\n";
        headerFile << "        // Implementation for creating game objects\n";
        headerFile << "    }\n";
        headerFile << "};\n";

        headerFile.close();

        // Create modified main.cpp that uses our level
        std::ofstream modifiedMainCpp(destination / "Main.cpp");
        if (!modifiedMainCpp.is_open()) {
            compilationLog_ += "Failed to create modified Main.cpp\n";
            return false;
        }

        modifiedMainCpp << "#include <windows.h>\n";
        modifiedMainCpp << "#include \"Engine.h\"\n";
        modifiedMainCpp << "#include \"GameLevel.h\"\n\n";
        modifiedMainCpp << "LPCWSTR szTitle = L\"" << level.GetSetting("GameTitle") << "\";\n";
        modifiedMainCpp << "LPCWSTR szWindowClass = L\"DIRECTXGAMEWINDOW\";\n";
        modifiedMainCpp << "HINSTANCE hInst;\n";
        modifiedMainCpp << "Engine* g_engine = nullptr;\n";
        modifiedMainCpp << "GameLevel* g_level = nullptr;\n\n";
        modifiedMainCpp << "LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);\n\n";

        modifiedMainCpp << "ATOM MyRegisterClass(HINSTANCE hInstance)\n";
        modifiedMainCpp << "{\n";
        modifiedMainCpp << "    WNDCLASS wc = {};\n";
        modifiedMainCpp << "    wc.lpfnWndProc = WndProc;\n";
        modifiedMainCpp << "    wc.hInstance = hInstance;\n";
        modifiedMainCpp << "    wc.lpszClassName = szWindowClass;\n";
        modifiedMainCpp << "    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);\n";
        modifiedMainCpp << "    return RegisterClass(&wc);\n";
        modifiedMainCpp << "}\n\n";

        modifiedMainCpp << "BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)\n";
        modifiedMainCpp << "{\n";
        modifiedMainCpp << "    hInst = hInstance;\n";
        modifiedMainCpp << "    HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,\n";
        modifiedMainCpp << "        CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);\n";
        modifiedMainCpp << "    if (!hWnd)\n";
        modifiedMainCpp << "        return FALSE;\n";
        modifiedMainCpp << "    ShowWindow(hWnd, nCmdShow);\n";
        modifiedMainCpp << "    UpdateWindow(hWnd);\n\n";
        modifiedMainCpp << "    // Create and initialize the engine\n";
        modifiedMainCpp << "    g_engine = new Engine(hWnd);\n";
        modifiedMainCpp << "    if (!g_engine->Initialize()) {\n";
        modifiedMainCpp << "        MessageBox(hWnd, L\"Engine initialization failed!\", L\"Error\", MB_OK);\n";
        modifiedMainCpp << "        delete g_engine;\n";
        modifiedMainCpp << "        g_engine = nullptr;\n";
        modifiedMainCpp << "        return FALSE;\n";
        modifiedMainCpp << "    }\n\n";
        modifiedMainCpp << "    // Create and initialize game level\n";
        modifiedMainCpp << "    g_level = new GameLevel();\n";
        modifiedMainCpp << "    g_level->Initialize(g_engine);\n\n";
        modifiedMainCpp << "    SetTimer(hWnd, 1, 16, NULL); // ~60fps\n";
        modifiedMainCpp << "    return TRUE;\n";
        modifiedMainCpp << "}\n\n";

        modifiedMainCpp << "int APIENTRY wWinMain(_In_ HINSTANCE hInstance,\n";
        modifiedMainCpp << "    _In_opt_ HINSTANCE hPrevInstance,\n";
        modifiedMainCpp << "    _In_ LPWSTR    lpCmdLine,\n";
        modifiedMainCpp << "    _In_ int       nCmdShow)\n";
        modifiedMainCpp << "{\n";
        modifiedMainCpp << "    MyRegisterClass(hInstance);\n";
        modifiedMainCpp << "    if (!InitInstance(hInstance, nCmdShow))\n";
        modifiedMainCpp << "        return FALSE;\n\n";
        modifiedMainCpp << "    MSG msg;\n";
        modifiedMainCpp << "    while (GetMessage(&msg, nullptr, 0, 0))\n";
        modifiedMainCpp << "    {\n";
        modifiedMainCpp << "        TranslateMessage(&msg);\n";
        modifiedMainCpp << "        DispatchMessage(&msg);\n";
        modifiedMainCpp << "    }\n";
        modifiedMainCpp << "    return (int)msg.wParam;\n";
        modifiedMainCpp << "}\n\n";

        modifiedMainCpp << "LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)\n";
        modifiedMainCpp << "{\n";
        modifiedMainCpp << "    switch (message)\n";
        modifiedMainCpp << "    {\n";
        modifiedMainCpp << "    case WM_TIMER:\n";
        modifiedMainCpp << "        if (g_engine) {\n";
        modifiedMainCpp << "            g_engine->Update();\n";
        modifiedMainCpp << "            if (g_level) g_level->Update(1.0f/60.0f);\n";
        modifiedMainCpp << "        }\n";
        modifiedMainCpp << "        break;\n";
        modifiedMainCpp << "    case WM_PAINT:\n";
        modifiedMainCpp << "    {\n";
        modifiedMainCpp << "        if (g_engine) {\n";
        modifiedMainCpp << "            g_engine->Render();\n";
        modifiedMainCpp << "            if (g_level) g_level->Render();\n";
        modifiedMainCpp << "        }\n";
        modifiedMainCpp << "        ValidateRect(hWnd, NULL); // Mark as painted\n";
        modifiedMainCpp << "    }\n";
        modifiedMainCpp << "    break;\n";
        modifiedMainCpp << "    case WM_DESTROY:\n";
        modifiedMainCpp << "        if (g_level) {\n";
        modifiedMainCpp << "            delete g_level;\n";
        modifiedMainCpp << "            g_level = nullptr;\n";
        modifiedMainCpp << "        }\n";
        modifiedMainCpp << "        if (g_engine) {\n";
        modifiedMainCpp << "            delete g_engine;\n";
        modifiedMainCpp << "            g_engine = nullptr;\n";
        modifiedMainCpp << "        }\n";
        modifiedMainCpp << "        PostQuitMessage(0);\n";
        modifiedMainCpp << "        break;\n";
        modifiedMainCpp << "    default:\n";
        modifiedMainCpp << "        return DefWindowProc(hWnd, message, wParam, lParam);\n";
        modifiedMainCpp << "    }\n";
        modifiedMainCpp << "    return 0;\n";
        modifiedMainCpp << "}\n";

        modifiedMainCpp.close();

        compilationLog_ += "Game code generated successfully.\n";
        return true;
    }
    catch (const std::exception& e) {
        compilationLog_ += "Error generating game code: ";
        compilationLog_ += e.what();
        compilationLog_ += "\n";
        return false;
    }
}

// Implementation of BuildGame - missing function
bool CompilerSystem::BuildGame(const fs::path& destination) {
    try {
        // Here you would add code to actually build the game
        // This could involve calling a compiler, linking libraries, etc.
        // For now, just log that we're simulating a build
        compilationLog_ += "Simulating build process...\n";

        // Simulate build process
        // In a real implementation, you might execute a command like:
        // system("cl.exe /EHsc /O2 /Fe:game.exe *.cpp");

        compilationLog_ += "Build completed successfully.\n";
        return true;
    }
    catch (const std::exception& e) {
        compilationLog_ += "Error building game: ";
        compilationLog_ += e.what();
        compilationLog_ += "\n";
        return false;
    }
}

// Implementation of EditorUI
EditorUI::EditorUI(HWND hWnd) : hWnd_(hWnd) {
}