//
// Created by Erfan Tavassoli on 4/13/26.
//

#ifndef CS_1D_BASEBALL_MAIN_PAGE_H
#define CS_1D_BASEBALL_MAIN_PAGE_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class main_page; }
QT_END_NAMESPACE

class main_page : public QWidget {
Q_OBJECT

public:
    explicit main_page(QWidget *parent = nullptr);
    ~main_page() override;

private:
    Ui::main_page *ui;
};


#endif //CS_1D_BASEBALL_MAIN_PAGE_H
