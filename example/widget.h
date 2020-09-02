#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <ScreenCapture/ScreenCapture.h>

#define SAFE_RELEASE(x) if(x) {x->Release(); x=NULL;}

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    void enumAdapter();
    void clearAdapter();

    static Buffer* Alloc(void* opaque, size_t size);
    static void Free(void* opaque, Buffer* buf);

private slots:
    void on_pushButton_Refresh_clicked();

    void on_pushButton_Start_clicked();

    void on_pushButton_Stop_clicked();

    void on_comboBox_Adapters_currentIndexChanged(int index);

    void takeSnapshot();

private:
    Ui::Widget *ui;
    QTimer* timer;

    ScreenCapture* sc;

    VideoAdapter* adapters;
    int adapter_count;
    VideoAdapter* selected_adapter;
    VideoOutput* selected_output;

};
#endif // WIDGET_H
