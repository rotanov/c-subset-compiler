#pragma once

#include <vector>

#include <QMainWindow>

class QLabel;

namespace Ui
{
    class MainWindow;
} // namespace Ui

enum CompilerMode
{
    CM_TOKENIZER,
    CM_SIMPLE_EXPRESSION,
};

const unsigned COMPILER_MODE_COUNT = 2;

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

    void OnQrbTokenizerToggled(bool value)
    {
        SetMode_(CM_TOKENIZER);
    }

    void OnQrbSimpleExpressionsToggled(bool value)
    {
        SetMode_(CM_SIMPLE_EXPRESSION);
    }

private slots:
    void on_action_New_triggered();
    void on_action_Save_triggered();
    void on_action_Next_triggered();
    void on_action_Prev_triggered();
    void on_action_Copy_Output_to_Reference_triggered();
    void on_action_Run_Tests_for_Current_Mode_triggered();
    void on_action_Tokenizer_triggered();
    void on_actionSimple_Expressions_triggered();
    void on_actionTest_Name_triggered();
    void OnQpteInputCursorPositionChanged();

    void on_action_Log_triggered(bool checked);

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow* ui = NULL;
    CompilerMode mode_ = CM_TOKENIZER;
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
