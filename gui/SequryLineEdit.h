#ifndef SEQURYLINEEDIT_H
#define SEQURYLINEEDIT_H

#include <QLineEdit>
#include <QKeyEvent>

class SequryLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    SequryLineEdit (QWidget *parent=nullptr) : QLineEdit(parent){ init(); }
    SequryLineEdit (const QString &contents, QWidget *parent=nullptr) : QLineEdit(contents,parent){ init(); }

private:
    void init(){
        setAcceptDrops(false);
        setContextMenuPolicy(Qt::CustomContextMenu);
        connect(this, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showMenu(QPoint)));
    }

protected:
    void keyPressEvent(QKeyEvent *event){
        if(event->matches(QKeySequence::Copy) || event->matches(QKeySequence::Cut) || event->matches(QKeySequence::Paste))
            event->ignore();
        else return QLineEdit::keyPressEvent(event);
    }

private slots:
    void showMenu(QPoint position){Q_UNUSED(position)}
};

#endif // SEQURYLINEEDIT_H
