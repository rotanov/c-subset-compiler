#include "MainWindow.hpp"
#include "ui_mainwindow.h"

#include <cassert>

#include <QTimer>
#include <QTextCodec>
#include <QLabel>
#include <QScrollBar>
#include <QFont>
#include <QObject>
#include <QRegExp>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QPalette>
#include <QRadioButton>
#include <QDir>
#include <QPushButton>
#include <QInputDialog>
#include <QFile>
#include <QTextStream>
#include <QSettings>

#include "constants.hpp"
#include "PreTokenizer.hpp"
#include "Tokenizer.hpp"
#include "DebugPreTokenStream.hpp"
#include "DebugTokenOutputStream.hpp"
#include "SimpleExpressionParser.hpp"
#include "ExpressionParser.hpp"
#include "Parser.hpp"

#include "CxxHighlighter.hpp"
#include "DebugStream.hpp"
#include "CodeEditor.hpp"

class QTextDocument;

void MainWindow::OnInputTextChanged()
{
    QString text = ui->qpteInput->toPlainText();
    ui->qpteOutput->clear();
    QByteArray utf8 = text.toUtf8();
    std::vector<char> input(utf8.size());
    for (int i = 0; i < utf8.size(); i++)
    {
        input[i] = utf8[i];
    }
    RunCompiler_(input);
    ui->qpteOutput->appendPlainText("");
}

void MainWindow::OnOutputTextChanged()
{
    CompareOutputWithReference_();
}

void MainWindow::OnReferenceTextChanged()
{
    CompareOutputWithReference_();
}

std::map<CompilerMode, std::string> CompilerModeToString =
{
    {CompilerMode::TOKENIZER, "Tokenizer"},
    {CompilerMode::SIMPLE_EXPRESSION, "Simple Expressions"},
    {CompilerMode::EXPRESSION_PARSER, "Expressions"},
    {CompilerMode::PARSER, "Parser"},
};

std::map<CompilerMode, std::string> CompilerModeToTestDir =
{
    {CompilerMode::TOKENIZER, "../tests/tokenizer/"},
    {CompilerMode::SIMPLE_EXPRESSION, "../tests/simple-expression-parser/"},
    {CompilerMode::EXPRESSION_PARSER, "../tests/expression-parser/"},
    {CompilerMode::PARSER, "../tests/parser/"},
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , cxxHighlighter_(NULL)

{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    ui->setupUi(this);
    qlStatus_ = new QLabel;
    qlLineColumn_ = new QLabel;
    ui->statusBar->addWidget(qlStatus_);
    ui->statusBar->addWidget(qlLineColumn_);

    int modeCount = static_cast<int>(CompilerMode::COUNT);
    tests_.resize(modeCount);
    testIndexes_.resize(modeCount, 0);
    for (const auto& i : CompilerModeToTestDir)
    {
        int index = static_cast<int>(i.first);
        std::vector<TestInfo>& tests = tests_[index];

        QDir dir(QString().fromStdString(i.second),
                 QString("*.t"),
                 QDir::Name | QDir::IgnoreCase,
                 QDir::Files);

        QFileInfoList entryInfoList = dir.entryInfoList();

        for (const auto& fileInfo : entryInfoList)
        {
            QString fileName = fileInfo.baseName();
            unsigned index = fileName.mid(0, 3).toInt();
            TestInfo testInfo;
            assert(tests.size() == index);
//            testInfo.name
            tests.push_back(testInfo);
        }
        if (tests.size() == 0)
        {
            tests.resize(1);
        }
    }



    ui->qpteInput->setFont(QFont("Consolas", 12));
    ui->qpteOutput->setFont(QFont("Consolas", 12));
    ui->qpteReference->setFont(QFont("Consolas", 12));
    ui->qpteLog->setFont(QFont("Consolas", 12));

    cxxHighlighter_ = new CxxHighlighter(ui->qpteInput->document());

    debugStreamCout_ = new DebugStream(std::cout, ui->qpteOutput);
    debugStreamCerr_ = new DebugStream(std::cerr, ui->qpteOutput);

    connect(ui->qpteInput, &QPlainTextEdit::textChanged,
            this, &MainWindow::OnInputTextChanged);
    connect(ui->qpteOutput, &QPlainTextEdit::textChanged,
            this, &MainWindow::OnOutputTextChanged);
    connect(ui->qpteReference, &QPlainTextEdit::textChanged,
            this, &MainWindow::OnReferenceTextChanged);

    connect(ui->qpteInput, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::OnQpteInputCursorPositionChanged);

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout,
            this, &MainWindow::onSyncScrollbars);
    timer->setInterval(200);
    timer->start();

    ui->qpteInput->verticalScrollBar()->setTracking(true);
    connect(ui->qpteInput->verticalScrollBar(), &QScrollBar::valueChanged,
            ui->qpteReference->verticalScrollBar(), &QScrollBar::setValue);

    ui->qpteOutput->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    ui->qpteReference->setLineWrapMode(QPlainTextEdit::WidgetWidth);

    SetMode_(mode_);
    UpdateTest_();
    showMaximized();

    ui->qpteLog->hide();

    on_action_Parser_triggered();
    on_action_Prev_triggered();
}

MainWindow::~MainWindow()
{
    delete cxxHighlighter_;
    delete debugStreamCout_;
    delete debugStreamCerr_;
    delete ui;
}

void MainWindow::CompareOutputWithReference_()
{
    QColor passColor = QColor(200, 255, 200);
    QColor failColor = QColor(255, 200, 200);

    QString outputText = ui->qpteOutput->toPlainText();
    QString referenceText = ui->qpteReference->toPlainText();

    QColor color = outputText == referenceText ? passColor : failColor;

    QPalette p = ui->qpteOutput->palette();
    p.setColor(QPalette::Active, QPalette::Base, color);
    p.setColor(QPalette::Inactive, QPalette::Base, color);
    ui->qpteOutput->setPalette(p);
    ui->qpteReference->setPalette(p);

//    QTextCharFormat tf;
//    tf.setBackground(QBrush(Qt::blue));
//    ui->qpteOutput->setCurrentCharFormat(tf);
}

void MainWindow::SetMode_(const CompilerMode &mode)
{
    mode_ = mode;
}

TestInfo &MainWindow::GetCurrentTest_()
{
    int modeIndex = static_cast<int>(mode_);
    return tests_[modeIndex][testIndexes_[modeIndex]];
}

void MainWindow::UpdateTest_()
{
    int modeIndex = static_cast<int>(mode_);
    QString filename;
    QTextStream filenameStream(&filename);
    filenameStream << QString().fromStdString(CompilerModeToTestDir[mode_])
                   << qSetFieldWidth(3)
                   << qSetPadChar('0')
                   << testIndexes_[modeIndex]
                   << qSetFieldWidth(0);
    QFile inputFile(filename + QString(".t"));
    assert(inputFile.open(QIODevice::ReadWrite | QIODevice::Text));
    ui->qpteInput->setPlainText(inputFile.readAll());
    QFile referenceFile(filename + QString(".ref"));
    assert(referenceFile.open(QIODevice::ReadWrite));
    ui->qpteReference->setPlainText(referenceFile.readAll());
    qlStatus_->setText(filename + ".t " + ui->qpteInput->textCursor().positionInBlock());

}

void MainWindow::RunCompiler_(std::vector<char> &input)
{
    using namespace Compiler;
    //        pretokenizer debug output
    //        DebugPreTokenStream debugPreTokenStream;
    //        PreTokenizer pretokenizer(input, debugPreTokenStream);

    ITokenStream* output = NULL;
    try
    {
        switch (mode_)
        {
            case CompilerMode::TOKENIZER:
            {
                output = new DebugTokenOutputStream;
                break;
            }

            case CompilerMode::SIMPLE_EXPRESSION:
            {
                output = new SimpleExpressionParser;
                break;
            }

            case CompilerMode::EXPRESSION_PARSER:
            {
                output = new ExpressionParser;
                break;
            }

            case CompilerMode::PARSER:
            {
                output = new Parser;
                break;
            }

            default:
            {
                throw std::runtime_error("unknown compiler mode");
                break;
            }
        }

        Tokenizer tokenizer(*output);
        PreTokenizer preTokenizer(input, tokenizer);
    }
    catch (std::exception& e)
    {
        output->Flush();
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    catch (boost::coroutines::detail::forced_unwind&)
    {
        throw;
    }
    catch (...)
    {
        std::cerr << "ERROR: unknown exception";
    }

    ui->qpteOutput->moveCursor(QTextCursor::Start);
    ui->qpteOutput->ensureCursorVisible();

    delete output;
}

void MainWindow::on_action_New_triggered()
{
    int modeIndex = static_cast<int>(mode_);
    TestInfo testInfo;
    testIndexes_[modeIndex] = tests_[modeIndex].size();
    tests_[modeIndex].push_back(testInfo);
    UpdateTest_();
}

void MainWindow::on_action_Save_triggered()
{
    int modeIndex = static_cast<int>(mode_);
    QString filename;
    QTextStream filenameStream(&filename);
    filenameStream << QString().fromStdString(CompilerModeToTestDir[mode_])
                   << qSetFieldWidth(3)
                   << qSetPadChar('0')
                   << testIndexes_[modeIndex]
                   << qSetFieldWidth(0);
    QFile fileInput(filename + ".t");
    assert(fileInput.open(QIODevice::ReadWrite | QIODevice::Truncate));
    fileInput.write(ui->qpteInput->toPlainText().toUtf8());
    QFile fileReference(filename + ".ref");
    assert(fileReference.open(QIODevice::ReadWrite | QIODevice::Truncate));
    fileReference.write(ui->qpteReference->toPlainText().toUtf8());
}

void MainWindow::on_action_Next_triggered()
{
    int modeIndex = static_cast<int>(mode_);
    testIndexes_[modeIndex] = (testIndexes_[modeIndex] + 1) % tests_[modeIndex].size();
    UpdateTest_();
}

void MainWindow::on_action_Prev_triggered()
{
    int modeIndex = static_cast<int>(mode_);
    testIndexes_[modeIndex] = (tests_[modeIndex].size() + testIndexes_[modeIndex] - 1) % tests_[modeIndex].size();
    UpdateTest_();
}

void MainWindow::on_action_Copy_Output_to_Reference_triggered()
{
    ui->qpteReference->clear();
    ui->qpteReference->setPlainText(ui->qpteOutput->toPlainText());
}

void MainWindow::on_action_Run_Tests_for_Current_Mode_triggered()
{
    if (!ui->qpteLog->isVisible())
    {
        ui->menu_View->actions().at(2)->trigger();
    }

    int modeIndex = static_cast<int>(mode_);

    QDir dir(QString().fromStdString(CompilerModeToTestDir[mode_]),
             QString("*.t"),
             QDir::Name | QDir::IgnoreCase,
             QDir::Files);

    QFileInfoList entryInfoList = dir.entryInfoList();

    ui->qpteLog->appendPlainText("");
    ui->qpteLog->appendPlainText(QString().fromStdString("Running tests for " + CompilerModeToString[mode_]));

    std::vector<int> failedTests;    
    for (unsigned i = 0; i < tests_[modeIndex].size(); i++)
    {
        QString filename;
        QTextStream filenameStream(&filename);
        filenameStream << QString().fromStdString(CompilerModeToTestDir[mode_])
                       << qSetFieldWidth(3)
                       << qSetPadChar('0')
                       << i
                       << qSetFieldWidth(0);

        QFile inputFile(filename + QString(".t"));
        QFile referenceFile(filename + QString(".ref"));
        assert(inputFile.open(QIODevice::ReadOnly | QIODevice::Text));
        assert(referenceFile.open(QIODevice::ReadOnly | QIODevice::Text));

        QByteArray inputByteArray = inputFile.readAll();
        std::vector<char> input;
        for (int i = 0; i < inputByteArray.size(); i++)
        {
            input.push_back(inputByteArray[i]);
        }
        QByteArray output;
        BufferStream* coutStream = new BufferStream(std::cout, output);
        BufferStream* cerrStream = new BufferStream(std::cerr, output);

        RunCompiler_(input);

        delete coutStream;
        delete cerrStream;

        QByteArray referenceByteArray = referenceFile.readAll();

        if (referenceByteArray.size() == output.size())
        {
            for (int i = 0; i < output.size(); i++)
            {
                if (referenceByteArray[i] != output[i])
                {
                    failedTests.push_back(i);
                    break;
                }
            }
        }
        else
        {
            failedTests.push_back(i);
        }
    }

    ui->qpteLog->appendPlainText(QString().fromStdString("Passed %1 of %2")
                                 .arg(tests_[modeIndex].size() - failedTests.size())
                                 .arg(tests_[modeIndex].size()));
    if (failedTests.size() == 0)
    {
        ui->qpteLog->appendPlainText("SUCCESS. ALL TESTS PASSED.");
    }
    else
    {
        QString failedTestsString;
        std::for_each(failedTests.begin(), failedTests.end(),
                      [&](int testIndex) { failedTestsString.append(QString().number(testIndex) + ", "); });
        ui->qpteLog->appendPlainText("FAIL. FAILED TESTS ARE: " + failedTestsString);
    }

}

void MainWindow::on_action_Tokenizer_triggered()
{
    SetMode_(CompilerMode::TOKENIZER);
    UpdateTest_();
}

void MainWindow::on_actionSimple_Expressions_triggered()
{
    SetMode_(CompilerMode::SIMPLE_EXPRESSION);
    UpdateTest_();
}

void MainWindow::on_actionTest_Name_triggered()
{
    QString testName = QInputDialog::getText(this, "Enter Test Name", "Test Name");
    GetCurrentTest_().name = testName;
}

void MainWindow::OnQpteInputCursorPositionChanged()
{
    QTextCursor cursor = ui->qpteInput->textCursor();
    int line = cursor.blockNumber() + 1;
    int column = cursor.columnNumber() + 1;
    std::string text = std::to_string(line) + "-" + std::to_string(column);
    qlLineColumn_->setText(QString().fromStdString(text));
}

void MainWindow::on_action_Log_triggered(bool checked)
{
    if (checked)
    {
        ui->qpteLog->show();
    }
    else
    {
        ui->qpteLog->hide();
    }
}

void MainWindow::on_action_Expressions_triggered()
{
    SetMode_(CompilerMode::EXPRESSION_PARSER);
    UpdateTest_();
}

void MainWindow::on_action_Parser_triggered()
{
    SetMode_(CompilerMode::PARSER);
    UpdateTest_();
}

void MainWindow::onSyncScrollbars()
{
//    ui->qpteReference->verticalScrollBar()->setValue(ui->qpteOutput->verticalScrollBar()->value());
    ui->qpteReference->setTextCursor(ui->qpteOutput->textCursor());
//    ui->qpteReference->ensureCursorVisible();
}

void MainWindow::on_actionInput_triggered()
{
    QList<int> list = {9999, 0, 0, 0};
    ui->splitter->setSizes(list);
}

void MainWindow::on_actionOutput_triggered()
{
    QList<int> list = {0, 9999, 0, 0};
    ui->splitter->setSizes(list);
}

void MainWindow::on_actionReference_triggered()
{
    QList<int> list = {0, 0, 9999, 0};
    ui->splitter->setSizes(list);
}

void MainWindow::on_actionLog_triggered()
{
    QList<int> list = {0, 0, 0, 9999};
    ui->splitter->setSizes(list);
}

void MainWindow::on_actionAll_equal_triggered()
{
    QList<int> list = {9999, 9999, 9999, 9999};
    ui->splitter->setSizes(list);
}
