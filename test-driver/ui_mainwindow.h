/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.1.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <CodeEditor.hpp>
#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *action_New;
    QAction *action_Save;
    QAction *action_Next;
    QAction *action_Prev;
    QAction *action_Copy_Output_to_Reference;
    QAction *action_Tokenizer;
    QAction *actionSimple_Expressions;
    QAction *action_Run_Tests_for_Current_Mode;
    QAction *actionTest_Name;
    QAction *action_Log;
    QAction *action_Expressions;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    QSplitter *splitter;
    CodeEditor *qpteInput;
    QPlainTextEdit *qpteOutput;
    QPlainTextEdit *qpteReference;
    QPlainTextEdit *qpteLog;
    QMenuBar *menuBar;
    QMenu *menu_File;
    QMenu *menu_Edit;
    QMenu *menu_Select_Mode;
    QMenu *menu_View;
    QMenu *menu_Run;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(1136, 835);
        action_New = new QAction(MainWindow);
        action_New->setObjectName(QStringLiteral("action_New"));
        action_Save = new QAction(MainWindow);
        action_Save->setObjectName(QStringLiteral("action_Save"));
        action_Next = new QAction(MainWindow);
        action_Next->setObjectName(QStringLiteral("action_Next"));
        action_Prev = new QAction(MainWindow);
        action_Prev->setObjectName(QStringLiteral("action_Prev"));
        action_Copy_Output_to_Reference = new QAction(MainWindow);
        action_Copy_Output_to_Reference->setObjectName(QStringLiteral("action_Copy_Output_to_Reference"));
        action_Tokenizer = new QAction(MainWindow);
        action_Tokenizer->setObjectName(QStringLiteral("action_Tokenizer"));
        actionSimple_Expressions = new QAction(MainWindow);
        actionSimple_Expressions->setObjectName(QStringLiteral("actionSimple_Expressions"));
        action_Run_Tests_for_Current_Mode = new QAction(MainWindow);
        action_Run_Tests_for_Current_Mode->setObjectName(QStringLiteral("action_Run_Tests_for_Current_Mode"));
        actionTest_Name = new QAction(MainWindow);
        actionTest_Name->setObjectName(QStringLiteral("actionTest_Name"));
        action_Log = new QAction(MainWindow);
        action_Log->setObjectName(QStringLiteral("action_Log"));
        action_Log->setCheckable(true);
        action_Expressions = new QAction(MainWindow);
        action_Expressions->setObjectName(QStringLiteral("action_Expressions"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        splitter = new QSplitter(centralWidget);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        splitter->setOpaqueResize(false);
        splitter->setChildrenCollapsible(false);
        qpteInput = new CodeEditor(splitter);
        qpteInput->setObjectName(QStringLiteral("qpteInput"));
        qpteInput->setLineWrapMode(QPlainTextEdit::NoWrap);
        splitter->addWidget(qpteInput);
        qpteOutput = new QPlainTextEdit(splitter);
        qpteOutput->setObjectName(QStringLiteral("qpteOutput"));
        qpteOutput->setUndoRedoEnabled(false);
        qpteOutput->setLineWrapMode(QPlainTextEdit::NoWrap);
        qpteOutput->setReadOnly(true);
        qpteOutput->setBackgroundVisible(false);
        splitter->addWidget(qpteOutput);
        qpteReference = new QPlainTextEdit(splitter);
        qpteReference->setObjectName(QStringLiteral("qpteReference"));
        qpteReference->setLineWrapMode(QPlainTextEdit::NoWrap);
        splitter->addWidget(qpteReference);
        qpteLog = new QPlainTextEdit(splitter);
        qpteLog->setObjectName(QStringLiteral("qpteLog"));
        qpteLog->setReadOnly(true);
        splitter->addWidget(qpteLog);

        verticalLayout->addWidget(splitter);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1136, 21));
        menu_File = new QMenu(menuBar);
        menu_File->setObjectName(QStringLiteral("menu_File"));
        menu_Edit = new QMenu(menuBar);
        menu_Edit->setObjectName(QStringLiteral("menu_Edit"));
        menu_Select_Mode = new QMenu(menu_Edit);
        menu_Select_Mode->setObjectName(QStringLiteral("menu_Select_Mode"));
        menu_View = new QMenu(menuBar);
        menu_View->setObjectName(QStringLiteral("menu_View"));
        menu_Run = new QMenu(menuBar);
        menu_Run->setObjectName(QStringLiteral("menu_Run"));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menu_File->menuAction());
        menuBar->addAction(menu_Edit->menuAction());
        menuBar->addAction(menu_View->menuAction());
        menuBar->addAction(menu_Run->menuAction());
        menu_File->addAction(action_New);
        menu_File->addAction(action_Save);
        menu_Edit->addAction(action_Copy_Output_to_Reference);
        menu_Edit->addAction(menu_Select_Mode->menuAction());
        menu_Edit->addAction(actionTest_Name);
        menu_Select_Mode->addAction(action_Tokenizer);
        menu_Select_Mode->addAction(actionSimple_Expressions);
        menu_Select_Mode->addAction(action_Expressions);
        menu_View->addAction(action_Prev);
        menu_View->addAction(action_Next);
        menu_View->addAction(action_Log);
        menu_Run->addAction(action_Run_Tests_for_Current_Mode);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        action_New->setText(QApplication::translate("MainWindow", "&New", 0));
        action_New->setShortcut(QApplication::translate("MainWindow", "Ctrl+Q", 0));
        action_Save->setText(QApplication::translate("MainWindow", "&Save", 0));
        action_Save->setShortcut(QApplication::translate("MainWindow", "Ctrl+S", 0));
        action_Next->setText(QApplication::translate("MainWindow", "&Next", 0));
        action_Next->setShortcut(QApplication::translate("MainWindow", "Ctrl+N", 0));
        action_Prev->setText(QApplication::translate("MainWindow", "&Prev", 0));
        action_Prev->setShortcut(QApplication::translate("MainWindow", "Ctrl+P", 0));
        action_Copy_Output_to_Reference->setText(QApplication::translate("MainWindow", "&Copy Output to Reference", 0));
        action_Copy_Output_to_Reference->setShortcut(QApplication::translate("MainWindow", "Ctrl+D", 0));
        action_Tokenizer->setText(QApplication::translate("MainWindow", "&Tokenizer", 0));
        actionSimple_Expressions->setText(QApplication::translate("MainWindow", "&Simple Expressions", 0));
        action_Run_Tests_for_Current_Mode->setText(QApplication::translate("MainWindow", "&Run Tests for Current Mode", 0));
        action_Run_Tests_for_Current_Mode->setShortcut(QApplication::translate("MainWindow", "Ctrl+R", 0));
        actionTest_Name->setText(QApplication::translate("MainWindow", "Test &Name", 0));
        actionTest_Name->setShortcut(QApplication::translate("MainWindow", "Ctrl+T", 0));
        action_Log->setText(QApplication::translate("MainWindow", "&Log", 0));
        action_Log->setShortcut(QApplication::translate("MainWindow", "Ctrl+L", 0));
        action_Expressions->setText(QApplication::translate("MainWindow", "&Expressions", 0));
        qpteOutput->setDocumentTitle(QApplication::translate("MainWindow", "Output", 0));
        menu_File->setTitle(QApplication::translate("MainWindow", "&File", 0));
        menu_Edit->setTitle(QApplication::translate("MainWindow", "&Edit", 0));
        menu_Select_Mode->setTitle(QApplication::translate("MainWindow", "&Select Mode", 0));
        menu_View->setTitle(QApplication::translate("MainWindow", "&View", 0));
        menu_Run->setTitle(QApplication::translate("MainWindow", "&Run", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
