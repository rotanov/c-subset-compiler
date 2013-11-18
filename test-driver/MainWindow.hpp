#pragma once

#include <vector>

#include <QMainWindow>

class QLabel;

namespace Ui
{
    class MainWindow;
} // namespace Ui

enum class CompilerMode
{
    TOKENIZER,
    SIMPLE_EXPRESSION,
    EXPRESSION_PARSER,
    PARSER,
    COUNT,
};

struct TestInfo
{
    QString name;
};

class DebugStream;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void OnInputTextChanged();
    void OnOutputTextChanged();
    void OnReferenceTextChanged();

private slots:
    void on_action_New_triggered();
    void on_action_Save_triggered();
    void on_action_Next_triggered();
    void on_action_Prev_triggered();
    void on_action_Copy_Output_to_Reference_triggered();
    void on_action_Run_Tests_for_Current_Mode_triggered();
    void on_action_Tokenizer_triggered();
    void on_actionSimple_Expressions_triggered();
    void on_action_Expressions_triggered();
    void on_actionTest_Name_triggered();
    void on_action_Log_triggered(bool checked);
    void OnQpteInputCursorPositionChanged();
    void on_action_Parser_triggered();
    void onSyncScrollbars();

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow* ui = NULL;
    CompilerMode mode_ = CompilerMode::TOKENIZER;
    std::vector<int> testIndexes_;
    std::vector<std::vector<TestInfo>> tests_;
    QLabel* qlStatus_ = NULL;
    QLabel* qlLineColumn_ = NULL;
    DebugStream* debugStreamCout_ = NULL; // YES WE CAN WHAT A RELIEF
    DebugStream* debugStreamCerr_ = NULL;

    void CompareOutputWithReference_();
    void SetMode_(const CompilerMode& mode);
    TestInfo& GetCurrentTest_();
    // reloads current mode_ current test
    void UpdateTest_();
    void RunCompiler_(std::vector<char>& input);
};
