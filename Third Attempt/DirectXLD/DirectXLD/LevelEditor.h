#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <filesystem>

namespace fs = std::filesystem;

// Forward declarations
class LevelObject;
class LevelData;
class EditorUI;
class CompilerSystem;

// Object types
enum class ObjectType {
    Mesh,
    Light,
    Camera,
    Trigger,
    Spawn
};

// Level object class
class LevelObject {
public:
    LevelObject(const std::string& name, ObjectType type);
    ~LevelObject();

    void SetPosition(float x, float y, float z);
    void SetRotation(float x, float y, float z);
    void SetScale(float x, float y, float z);
    void SetProperty(const std::string& key, const std::string& value);
    std::string GetProperty(const std::string& key);

    std::string GetName() const { return name_; }
    ObjectType GetType() const { return type_; }
    const float* GetPosition() const { return position_; }
    const float* GetRotation() const { return rotation_; }
    const float* GetScale() const { return scale_; }

    void Serialize(std::ofstream& file);
    static std::unique_ptr<LevelObject> Deserialize(std::ifstream& file);

private:
    std::string name_;
    ObjectType type_;
    float position_[3];
    float rotation_[3];
    float scale_[3];
    std::map<std::string, std::string> properties_;
};

// Level data class
class LevelData {
public:
    LevelData();
    ~LevelData();

    void AddObject(std::unique_ptr<LevelObject> object);
    void RemoveObject(const std::string& name);
    LevelObject* GetObject(const std::string& name);
    std::vector<LevelObject*> GetObjectsByType(ObjectType type) const;

    void SetSetting(const std::string& key, const std::string& value);
    std::string GetSetting(const std::string& key) const;

    bool SaveToFile(const fs::path& path);
    bool LoadFromFile(const fs::path& path);

    const std::vector<std::unique_ptr<LevelObject>>& GetObjects() const { return objects_; }

private:
    std::vector<std::unique_ptr<LevelObject>> objects_;
    std::map<std::string, std::string> settings_;
};

// Compiler system for creating game builds
class CompilerSystem {
public:
    CompilerSystem();
    ~CompilerSystem();

    bool Initialize(const fs::path& enginePath, const fs::path& templatePath, const fs::path& outputPath);
    bool CompileLevel(const LevelData& level, const std::string& gameName);
    const std::string& GetCompilationLog() const { return compilationLog_; }
    bool IsCompiling() const { return isCompiling_; }

private:
    bool CopyEngineFiles(const fs::path& destination);
    bool GenerateGameCode(const LevelData& level, const fs::path& destination);
    bool BuildGame(const fs::path& destination);

    fs::path enginePath_;
    fs::path templatePath_;
    fs::path outputPath_;
    std::string compilationLog_;
    bool isCompiling_;
};

// Editor UI class
class EditorUI {
public:
    EditorUI(HWND hWnd);
    ~EditorUI();

    bool Initialize();
    void Render();
    void Shutdown();

    void RenderToolbar();
    void RenderViewport();
    void RenderObjectPanel();
    void RenderPropertiesPanel();
    void RenderCompilerPanel();

    void OnObjectSelected(const std::string& objectName);
    void OnPropertyChanged(const std::string& propertyName, const std::string& propertyValue);

private:
    HWND hWnd_;
    // Add UI state members here
};

// Main Level Editor class
class LevelEditor {
public:
    LevelEditor(HINSTANCE hInstance);
    ~LevelEditor();

    bool Initialize();
    bool Run();
    void Shutdown();

    HWND GetMainWindow() const { return hWnd_; }
    LevelData& GetLevelData() { return levelData_; }
    EditorUI& GetEditorUI() { return *editorUI_; }
    CompilerSystem& GetCompilerSystem() { return *compilerSystem_; }

    // Event handlers
    void OnNewLevel();
    void OnSaveLevel();
    void OnLoadLevel();
    void OnCompileLevel();

    // Window procedure
    static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT ProcessWindowMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    bool RegisterWindowClass();
    HWND CreateEditorWindow();
    void SetupMenus();
    bool ProcessMessage();

    HINSTANCE hInstance_;
    HWND hWnd_;
    LevelData levelData_;
    std::unique_ptr<EditorUI> editorUI_;
    std::unique_ptr<CompilerSystem> compilerSystem_;
    bool isRunning_;

    // Resource IDs - moved to this header to prevent redefinition
    static const int IDM_NEW = 1001;
    static const int IDM_OPEN = 1002;
    static const int IDM_SAVE = 1003;
    static const int IDM_COMPILE = 1004;
    static const int IDM_EXIT = 1005;
    static const int IDM_ABOUT = 1006;

    static const int ID_TOOLBAR = 2001;
    static const int ID_STATUSBAR = 2002;
    static const int ID_OBJECTLIST = 2003;
    static const int ID_PROPERTYLIST = 2004;
};