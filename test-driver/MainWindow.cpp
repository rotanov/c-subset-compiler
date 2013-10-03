#include "MainWindow.hpp"
#include "ui_mainwindow.h"

#include <cassert>

#include <QTextCodec>
#include <QLabel>
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

#include "CxxHighlighter.hpp"
#include "DebugStream.hpp"
#include "CodeEditor.hpp"

class QTextDocument;

void MainWindow::OnInputTextChanged()
{
    try
    {
        using namespace Compiler;
        QString text = ui->qpteInput->toPlainText();
        ui->qpteOutput->clear();
        QByteArray utf8 = text.toUtf8();
        vector<char> input(utf8.size());
        for (int i = 0; i < utf8.size(); i++)
        {
            input[i] = utf8[i];
        }

    //        pretokenizer debug output
    //        DebugPreTokenStream debugPreTokenStream;
    //        PreTokenizer pretokenizer(input, debugPreTokenStream);


        switch (mode_)
        {
        case CM_TOKENIZER:
        {
            DebugTokenOutputStream debugTokenOutputStream;
            Tokenizer tokenizer(debugTokenOutputStream);
            PreTokenizer pretokenizer(input, tokenizer);
        }
            break;

        case CM_SIMPLE_EXPRESSION:
        {
            SimpleExpressionParser simpleExpressionParser;
            Tokenizer tokenizer(simpleExpressionParser);
            PreTokenizer pretokenizer(input, tokenizer);
        }
            break;
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
//        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "ERROR: unknown exception";
//        return EXIT_FAILURE;
    }
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
    { CM_TOKENIZER, "Tokenizer" },
    { CM_SIMPLE_EXPRESSION, "Simple Expressions" },
};

std::map<CompilerMode, std::string> CompilerModeToTestDir =
{
    { CM_TOKENIZER, "../tests/tokenizer/" },
    { CM_SIMPLE_EXPRESSION, "../tests/simple-expression-parser/" },
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , mode_(CM_TOKENIZER)
    , qlStatus_(NULL)

{
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));

    ui->setupUi(this);
    qlStatus_ = new QLabel;
    ui->statusBar->addWidget(qlStatus_);

    tests_.resize(COMPILER_MODE_COUNT);
    testIndexes_.resize(COMPILER_MODE_COUNT, 0);
    for (const auto& i : CompilerModeToTestDir)
    {
        std::vector<TestInfo>& tests = tests_[i.first];
        int& testIndex = testIndexes_[i.first];

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

    CxxHighlighter* cxxHighlighter = new CxxHighlighter(ui->qpteInput->document());

    DebugStream* debugStreamCout = new DebugStream(std::cout, ui->qpteOutput);
    DebugStream* debugStreamCerr = new DebugStream(std::cerr, ui->qpteOutput);

    connect(ui->qpteInput, &QPlainTextEdit::textChanged,
            this, &MainWindow::OnInputTextChanged);
    connect(ui->qpteOutput, &QPlainTextEdit::textChanged,
            this, &MainWindow::OnOutputTextChanged);
    connect(ui->qpteReference, &QPlainTextEdit::textChanged,
            this, &MainWindow::OnReferenceTextChanged);

    SetMode_(mode_);
    UpdateTest_();
    showMaximized();
}

MainWindow::~MainWindow()
{
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
}

void MainWindow::SetMode_(const CompilerMode &mode)
{
    mode_ = mode;
}

TestInfo &MainWindow::GetCurrentTest_()
{
    return tests_[mode_][testIndexes_[mode_]];
}

void MainWindow::UpdateTest_()
{
    QString filename;
    QTextStream filenameStream(&filename);
    filenameStream << QString().fromStdString(CompilerModeToTestDir[mode_])
                   << qSetFieldWidth(3)
                   << qSetPadChar('0')
                   << testIndexes_[mode_]
                   << qSetFieldWidth(0);
    QFile inputFile(filename + QString(".t"));
    assert(inputFile.open(QIODevice::ReadWrite | QIODevice::Text));
    ui->qpteInput->setPlainText(inputFile.readAll());
    QFile referenceFile(filename + QString(".ref"));
    assert(referenceFile.open(QIODevice::ReadWrite));
    ui->qpteReference->setPlainText(referenceFile.readAll());
    qlStatus_->setText(filename + ".t " + ui->qpteInput->textCursor().positionInBlock());

}

void MainWindow::on_action_New_triggered()
{
    TestInfo testInfo;
    testIndexes_[mode_] = tests_[mode_].size();
    tests_[mode_].push_back(testInfo);
    UpdateTest_();
}

void MainWindow::on_action_Save_triggered()
{
    QString filename;
    QTextStream filenameStream(&filename);
    filenameStream << QString().fromStdString(CompilerModeToTestDir[mode_])
                   << qSetFieldWidth(3)
                   << qSetPadChar('0')
                   << testIndexes_[mode_]
                   << qSetFieldWidth(0);
    QFile fileInput(filename + ".t");
    assert(fileInput.open(QIODevice::ReadWrite));
    fileInput.write(ui->qpteInput->toPlainText().toUtf8());
    QFile fileReference(filename + ".ref");
    assert(fileReference.open(QIODevice::ReadWrite));
    fileReference.write(ui->qpteReference->toPlainText().toUtf8());
}

void MainWindow::on_action_Next_triggered()
{
    testIndexes_[mode_] = (testIndexes_[mode_] + 1) % tests_[mode_].size();
    UpdateTest_();
}

void MainWindow::on_action_Prev_triggered()
{
    testIndexes_[mode_] = (tests_[mode_].size() + testIndexes_[mode_] - 1) % tests_[mode_].size();
    UpdateTest_();
}

void MainWindow::on_action_Copy_Output_to_Reference_triggered()
{
    ui->qpteReference->setPlainText(ui->qpteOutput->toPlainText());
}

void MainWindow::on_action_Run_Tests_for_Current_Mode_triggered()
{

}

void MainWindow::on_action_Tokenizer_triggered()
{
    SetMode_(CM_TOKENIZER);
    UpdateTest_();
}

void MainWindow::on_actionSimple_Expressions_triggered()
{
    SetMode_(CM_SIMPLE_EXPRESSION);
    UpdateTest_();
}

void MainWindow::on_actionTest_Name_triggered()
{
    QString testName = QInputDialog::getText(this, "Enter Test Name", "Test Name");
    GetCurrentTest_().name = testName;
}
