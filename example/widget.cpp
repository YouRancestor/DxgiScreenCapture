#include "widget.h"
#include "ui_widget.h"

#include <QTimer>

class MyBuffer : public Buffer
{
public:
    MyBuffer(uint32_t width, uint32_t height, size_t size)
        : img(width, height, QImage::Format_ARGB32)
    {
        const uchar* constdata = img.bits();
        this->ptr = (uint8_t*)constdata;
        assert(size <= img.sizeInBytes());
        this->size = img.sizeInBytes();
    }
    ~MyBuffer(){}

    QImage GetImage() {return img;}

private:
    QImage img;
};

Buffer* Widget::Alloc(void* opaque, size_t size)
{
    Widget* self = (Widget*)opaque;
    uint32_t width = self->selected_output->width;
    uint32_t height = self->selected_output->height;
    Buffer* buf = new MyBuffer(width, height, size);
    return buf;
}
void Widget::Free(void* opaque, Buffer* buf)
{
    (void)(opaque);
    MyBuffer* mybuf = (MyBuffer*)buf;
    delete mybuf;
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , timer(new QTimer)
    , sc(NULL)
    , adapters(NULL)
    , adapter_count(0)
    , selected_adapter(NULL)
    , selected_output(NULL)
{
    ui->setupUi(this);
    enumAdapter();
    timer->setInterval(30);
    connect(timer, SIGNAL(timeout()), this, SLOT(takeSnapshot()), Qt::QueuedConnection);

}

Widget::~Widget()
{
    on_pushButton_Stop_clicked();
    clearAdapter();
    delete ui;
}

void Widget::enumAdapter()
{
    EnumerateAdaptersAndOutputs(&adapters, &adapter_count);
    for (int i = 0; i < adapter_count; ++i) {
        ui->comboBox_Adapters->addItem(QString::fromWCharArray(adapters[i].desc));
    }

}

void Widget::clearAdapter()
{
    ui->comboBox_Adapters->clear();
    if (adapters)
    {
        FreeVideoAdapters(adapters);
        adapters = NULL;
    }
}

void Widget::on_pushButton_Refresh_clicked()
{
    clearAdapter();
    enumAdapter();
}

void Widget::on_pushButton_Start_clicked()
{
    on_pushButton_Stop_clicked();

    selected_adapter = &adapters[ui->comboBox_Adapters->currentIndex()];
    selected_output = &selected_adapter->outputs[ui->comboBox_Outputs->currentIndex()];

    CreateScreenCapture(ui->comboBox_Adapters->currentIndex(), ui->comboBox_Outputs->currentIndex(), &sc);
    SetMemAllocator(sc, Alloc, Free, this);

    takeSnapshot();
    timer->start();
}

void Widget::on_pushButton_Stop_clicked()
{
    timer->stop();
    if (sc)
    {
        DestroyScreenCapture(sc);
        sc = NULL;
    }

    ui->comboBox_Outputs->setEnabled(true);
    ui->comboBox_Adapters->setEnabled(true);

    selected_adapter = NULL;
    selected_output = NULL;
}

void Widget::on_comboBox_Adapters_currentIndexChanged(int index)
{
    ui->comboBox_Outputs->clear();
    if (!adapters)
        return;
    if (index < 0)
        return;
    for (uint32_t i = 0; i < adapters[index].output_count; ++i) {
        ui->comboBox_Outputs->addItem(QString::fromWCharArray(adapters[index].outputs[i].name));
    }
}


void Widget::takeSnapshot()
{
    Frame *frm = 0;
    TakeSnapshot(sc, 0, &frm);
    if (!frm)
        return;
    ui->label->setPixmap(QPixmap::fromImage(((MyBuffer*)frm->buffer)->GetImage()));
    FrameRelease(sc, frm);
    update();
}
