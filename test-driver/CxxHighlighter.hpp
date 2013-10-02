#pragma once

#include <QSyntaxHighlighter>

class CxxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    CxxHighlighter(QTextDocument *parent = 0);
    virtual ~CxxHighlighter()
    {

    }

protected:
    void highlightBlock(const QString &text);

private:
    struct HighlightingRule
    {
        QRegExp pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;

    QRegExp commentStartExpression;
    QRegExp commentEndExpression;

    QTextCharFormat keywordFormat;
    QTextCharFormat classFormat;
    QTextCharFormat singleLineCommentFormat;
    QTextCharFormat multiLineCommentFormat;
    QTextCharFormat quotationFormat;
    QTextCharFormat functionFormat;
};
