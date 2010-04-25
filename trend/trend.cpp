#include "trend.h"
#include "ui_trend.h"

#include <QColor>
#include <QPen>
#include <QPixmap>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QColorDialog>
#include <QDebug>

#include "math.h"

#include "datetimedialog.h"

#include <QHeaderView>
#include <QTableWidgetItem>

// Функції-члени класу TrendWindow

#define TR_HEIGHT 10000

namespace MyConst
{
    const int tmr[]={900,1800,3600,7200,14400,21600,43200};
    const unsigned int LvCor = 2049136799;
    const Qt::GlobalColor DefColor[8]={Qt::red,Qt::green,Qt::blue,Qt::cyan,Qt::magenta,Qt::yellow,Qt::white,Qt::darkRed};
};

TrendWindow::TrendWindow(QWidget *p,struct trendinfo *tri,int nHeight) : QWidget(p)
    ,m_nHeight(nHeight),m_pnDt(NULL),m_ui(new Ui::Trend)
{    
    //qDebug("Настройка вікна");

    m_ui->setupUi(this);
    //setAttribute( Qt::WA_DeleteOnClose); // ??? це що таке
    connect(m_ui->Exit,SIGNAL(clicked()),this,SLOT(slotExit()));

    m_trinfo=tri;
    setObjectName(m_trinfo->trend);    
    
    QVBoxLayout *vbl=new QVBoxLayout(m_ui->trend); // менеджер розміщення для об'єкту TrendView
    m_tw=new TrendView(m_ui->trend);
    vbl->setContentsMargins(2,2,2,2);
    vbl->addWidget(m_tw);    
    m_ui->trend->setLayout(vbl);
    
    connect(m_tw,SIGNAL(cursorMoved(int)),this,SLOT(setCursor(int)));
    
    m_ui->trendHead->setText(m_trinfo->trendHead);

    // навігаційні кнопки
    connect(m_ui->forward,SIGNAL(clicked()),this,SLOT(dataChange()));
    connect(m_ui->backward,SIGNAL(clicked()),this,SLOT(dataChange()));
    connect(m_ui->forwardHalf,SIGNAL(clicked()),this,SLOT(dataChange()));
    connect(m_ui->backwardHalf,SIGNAL(clicked()),this,SLOT(dataChange()));
    connect(m_ui->calendarButton,SIGNAL(clicked()),this,SLOT(dataChange()));
    connect(m_ui->last,SIGNAL(clicked()),this,SLOT(dataChange()));
    connect(m_ui->Interval,SIGNAL(activated(int)),this,SLOT(dataChange()));
    // кнопки вибору кольору
    connect(m_ui->pc_0,SIGNAL(clicked()),this,SLOT(colorChange()));
    connect(m_ui->pc_1,SIGNAL(clicked()),this,SLOT(colorChange()));
    connect(m_ui->pc_2,SIGNAL(clicked()),this,SLOT(colorChange()));
    connect(m_ui->pc_3,SIGNAL(clicked()),this,SLOT(colorChange()));
    connect(m_ui->pc_4,SIGNAL(clicked()),this,SLOT(colorChange()));
    connect(m_ui->pc_5,SIGNAL(clicked()),this,SLOT(colorChange()));
    connect(m_ui->pc_6,SIGNAL(clicked()),this,SLOT(colorChange()));
    connect(m_ui->pc_7,SIGNAL(clicked()),this,SLOT(colorChange()));
    // перемикач вибору поля курсоку
    connect(m_ui->ps_0,SIGNAL(clicked()),this,SLOT(plotChange()));
    connect(m_ui->ps_1,SIGNAL(clicked()),this,SLOT(plotChange()));
    connect(m_ui->ps_2,SIGNAL(clicked()),this,SLOT(plotChange()));
    connect(m_ui->ps_3,SIGNAL(clicked()),this,SLOT(plotChange()));
    connect(m_ui->ps_4,SIGNAL(clicked()),this,SLOT(plotChange()));
    connect(m_ui->ps_5,SIGNAL(clicked()),this,SLOT(plotChange()));
    connect(m_ui->ps_6,SIGNAL(clicked()),this,SLOT(plotChange()));
    connect(m_ui->ps_7,SIGNAL(clicked()),this,SLOT(plotChange()));
    // чекбокси вмикання графіку
    connect(m_ui->pv_0,SIGNAL(clicked()),this,SLOT(dataChange()));
    connect(m_ui->pv_1,SIGNAL(clicked()),this,SLOT(dataChange()));
    connect(m_ui->pv_2,SIGNAL(clicked()),this,SLOT(dataChange()));
    connect(m_ui->pv_3,SIGNAL(clicked()),this,SLOT(dataChange()));
    connect(m_ui->pv_4,SIGNAL(clicked()),this,SLOT(dataChange()));
    connect(m_ui->pv_5,SIGNAL(clicked()),this,SLOT(dataChange()));
    connect(m_ui->pv_6,SIGNAL(clicked()),this,SLOT(dataChange()));
    connect(m_ui->pv_7,SIGNAL(clicked()),this,SLOT(dataChange()));
    // сигнал від перегрядача трендів, запит наперемалювання
    connect(m_tw,SIGNAL(_redraw()),this,SLOT(dataChange()));
    
    //connect(m_ui->cursorVal,SIGNAL(valueChanged(int)),m_ui->cursorLCD,SLOT(display(int)));
        
    //фарбую кнопки
    QPalette pal;
    
    pal.setColor(QPalette::Button,QColor(MyConst::DefColor[0]));
    m_ui->pc_0->setPalette(pal);

    pal.setColor(QPalette::Button,QColor(MyConst::DefColor[1]));
    m_ui->pc_1->setPalette(pal);
    
    pal.setColor(QPalette::Button,QColor(MyConst::DefColor[2]));
    m_ui->pc_2->setPalette(pal);

    pal.setColor(QPalette::Button,QColor(MyConst::DefColor[3]));
    m_ui->pc_3->setPalette(pal);

    pal.setColor(QPalette::Button,QColor(MyConst::DefColor[4]));
    m_ui->pc_4->setPalette(pal);

    pal.setColor(QPalette::Button,QColor(MyConst::DefColor[5]));
    m_ui->pc_5->setPalette(pal);

    pal.setColor(QPalette::Button,QColor(MyConst::DefColor[6]));
    m_ui->pc_6->setPalette(pal);

    pal.setColor(QPalette::Button,QColor(MyConst::DefColor[7]));
    m_ui->pc_7->setPalette(pal);

    QPalette pl;
    pl.setColor(QPalette::Highlight,QColor(MyConst::DefColor[0]));
    m_ui->cursorVal->setPalette(pl);

    m_ui->cursorVal->setValue(0);
    m_ui->cursorVal->setRange(0,m_nHeight);
    
    QPalette pn;     
    pn.setColor(QPalette::Window,QColor((QRgb)0x84A3AB));
    
    m_ui->frame->setAutoFillBackground(true);
    m_ui->frame->setPalette(pn);

    m_ui->navigation->setAutoFillBackground(true);
    m_ui->navigation->setPalette(pn);

    m_ui->view->setAutoFillBackground(true);
    m_ui->view->setPalette(pn);

    m_ui->ps_0->setChecked(true);
    
    m_ui->pv_0->setChecked(true);
    m_ui->pv_1->setChecked(true);
    m_ui->pv_2->setChecked(true);
    m_ui->pv_3->setChecked(true);
    m_ui->pv_4->setChecked(true);
    m_ui->pv_5->setChecked(true);
    m_ui->pv_6->setChecked(true);
    m_ui->pv_7->setChecked(true);
    
    pv[0]=m_ui->pv_0;
    pv[1]=m_ui->pv_1;
    pv[2]=m_ui->pv_2;
    pv[3]=m_ui->pv_3;
    pv[4]=m_ui->pv_4;
    pv[5]=m_ui->pv_5;
    pv[6]=m_ui->pv_6;
    pv[7]=m_ui->pv_7;
    
    ps[0]=m_ui->ps_0;
    ps[1]=m_ui->ps_1;
    ps[2]=m_ui->ps_2;
    ps[3]=m_ui->ps_3;
    ps[4]=m_ui->ps_4;
    ps[5]=m_ui->ps_5;
    ps[6]=m_ui->ps_6;
    ps[7]=m_ui->ps_7;
    
    
    for(int i=0;i<8;++i)
	m_Color[i]=QColor(MyConst::DefColor[i]); // цю ініціалізацію треба переробити

    m_tw->setColors(m_Color);

    m_stop=QDateTime::currentDateTime();

    // таблиця параметрів внизу вікна
    m_ui->twAgr->setColumnCount(4);
    m_ui->twAgr->setRowCount(8);
    m_ui->twAgr->setVerticalHeaderLabels(m_trinfo->fieldHead);
    m_ui->twAgr->horizontalHeader()->hide();
    m_ui->twAgr->verticalHeader()->setFixedWidth(300);
    m_ui->twAgr->setSortingEnabled(false);
    for(int i=0;i<8;++i)
        m_ui->twAgr->setRowHeight(i,20);
    for(int i=0;i<4;++i)
        m_ui->twAgr->setColumnWidth(i,75);
    QTableWidgetItem *_item;
    for(int i=0;i<8;++i)
    {
	for(int j=0;j<4;++j)
	{
	    _item= new QTableWidgetItem;
	    _item->setText("0.00");
            m_ui->twAgr->setItem(i,j,_item);
	}
    }
    
    m_sTmpl= "SELECT Dt%1 FROM %2 WHERE Dt BETWEEN %3 AND %4 ORDER BY Dt";// шаблон запиту

    QSqlDatabase dbs=QSqlDatabase::addDatabase("QMYSQL","history");
    QSettings s;

    dbs.setHostName(tri->host);
    dbs.setDatabaseName(tri->db);
    dbs.setUserName(tri->user);
    dbs.setPassword(tri->passwd);

    if(dbs.open())
    {
	//qDebug() << "Connect to DB.";

	QSqlQuery query(dbs);
	// визначення чісового інтервалу наявних даних
	if(query.exec(QString("SELECT min(Dt) ,max(Dt) from %1").arg(m_trinfo->table)))
	{
	    QDateTime tmp;
	    query.next();
	    
	    //qDebug("min(Dt)=%u",query.value(0).toUInt());
	    //qDebug("max(Dt)=%u",query.value(1).toUInt());	    
	    
	    tmp.setTime_t(query.value(0).toUInt());
            m_ui->db_startDate->setText(tmp.toString("hh:mm:ss\ndd:MM:yy"));

	    tmp.setTime_t(query.value(1).toUInt());
            m_ui->db_stopDate->setText(tmp.toString("hh:mm:ss\ndd:MM:yy"));
	    query.clear();
	}
	else
	{
	    //qDebug() << query.lastError();
	}
	// перемалювання графіку
        //dataChange();
	//setCursor(0);

    }
    else
    {
	// якщо не зв’язалися із БД
	//qDebug() << dbs.lastError();
    }
    //}
}

TrendWindow::~TrendWindow()
{
    delete m_pnDt; 
    //qDebug("TrendWindow Кінець роботи");
    {
	QSqlDatabase dbs=QSqlDatabase::database("history");
	dbs.close();
    }
    QSqlDatabase::removeDatabase("history");
    delete m_ui;

}

void TrendWindow::slotExit()
{
    emit finished();
}

void TrendWindow::dataChange()
{
    //qDebug() << "start" << sender();
    QString s="";
    int i ;
    
    if(sender()!=NULL) // обробка зміни часу від навігаційних кнопок
    {
	QString sndr=sender()->objectName();
	if(sndr=="last")
	{
	    m_stop=QDateTime::currentDateTime();
	}
	else if(sndr=="forward")
	{
            m_stop=m_stop.addSecs(MyConst::tmr[m_ui->Interval->currentIndex()]);
	}
	else if(sndr=="forwardHalf")
	{
            m_stop=m_stop.addSecs(MyConst::tmr[m_ui->Interval->currentIndex()]/2);
	}
	else if(sndr=="backwardHalf")
	{
            m_stop=m_stop.addSecs(-MyConst::tmr[m_ui->Interval->currentIndex()]/2);
	}
	else if(sndr=="backward")
	{
            m_stop=m_stop.addSecs(-MyConst::tmr[m_ui->Interval->currentIndex()]);
	}
	else if(sndr=="calendarButton")
	{
	    myDateTimeDialog dt(this,m_stop);
	    if(dt.exec()==QDialog::Accepted)
	    {
		m_stop.setDate(dt.date->selectedDate());
		m_stop.setTime(QTime(dt.hour->value(),dt.minute->value(),dt.second->value()));
	    }
	}
    }

    // початок виконання запиту та малювання графіку
    QApplication::setOverrideCursor(Qt::WaitCursor);
    
    m_start=m_stop.addSecs(-MyConst::tmr[m_ui->Interval->currentIndex()]);

    m_ui->startDate->setText(m_start.toString("hh:mm:ss\ndd:MM:yy"));
    m_ui->stopDate->setText(m_stop.toString("hh:mm:ss\ndd:MM:yy"));

    for(i=0;i<m_trinfo->numPlot;++i)  //визначення полів запиту. ті які не відображаються == -1 щоб бути поза зоною відображення
    {	
	s+=",";	
	s+=pv[i]->checkState()==Qt::Checked?m_trinfo->fields[i]:"-1";
    }
    //qDebug() << "FL s=" << s << "m_trinfo->numPlot = " << m_trinfo->numPlot;
    
    // фінальна побудова запиту
    s=m_sTmpl.arg(s).arg(m_trinfo->table).arg(m_start.toTime_t()).arg(m_stop.toTime_t());

QSqlDatabase dbs=QSqlDatabase::database("history");
if(dbs.isOpen())
{
    QSqlQuery query(dbs);

    //qDebug() << s;
    if(query.exec(s))
    {
	//qDebug() << m_trinfo->numPlot;
	//int *v= new int[m_trinfo->numPlot];
	QVector<int> v(m_trinfo->numPlot);
	//qDebug("Кількість записів %d",query.size());
	if(m_pnDt)
	    delete m_pnDt;
	m_pnDt = new unsigned int[query.size()];
	// ініціалізація об'єкта малювання графіку
	m_tw->start(m_stop.toTime_t()-m_start.toTime_t(),m_trinfo->numPlot,m_nHeight);
	// виймання даних
	m_nLen=0;
	while(query.next())
	{
	    m_pnDt[m_nLen++]=query.value(0).toUInt(); // зберегти масив точок
	    
	    for(i=0;i<m_trinfo->numPlot;++i)
		v[i]=query.value(i+1).toInt();
	    m_tw->setData(query.value(0).toUInt()-m_start.toTime_t(),v);
	}

	query.clear();
	m_tw->draw();
	
	//delete v;
	QApplication::setOverrideCursor(Qt::ArrowCursor);
	setCursor(-1);
    }
    else
    {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
	QMessageBox::critical(this,tr("Помилка виконання запиту"),query.lastError().databaseText());
    }        
    
    s.clear();
    for(i=0;i<m_trinfo->numPlot;++i)
    {
	s+=pv[i]->checkState()==Qt::Checked?",min("+m_trinfo->fields[i]+"),avg("+m_trinfo->fields[i]+"),max("+m_trinfo->fields[i]+")" :",-1,-1,-1";
    }
    s=QString("SELECT COUNT(Dt)%1 FROM %2 WHERE Dt BETWEEN %3 AND %4").arg(s).arg(m_trinfo->table).arg(m_start.toTime_t()).arg(m_stop.toTime_t());
//    qDebug() << s;
    
    if(query.exec(s))
    {
	double min,avg,max;
	
	query.next();

	for(i=0;i<m_trinfo->numPlot;++i)
	{
	    min=query.value(i*3+1).toDouble() *(m_trinfo->fScale[i][1]-m_trinfo->fScale[i][0])/m_nHeight + m_trinfo->fScale[i][0];
            m_ui->twAgr->item(i,1)->setText(QString("%1").arg(min,6,'f',2));
	    	    
	    avg=query.value(i*3+2).toDouble() *(m_trinfo->fScale[i][1]-m_trinfo->fScale[i][0])/m_nHeight + m_trinfo->fScale[i][0];
            m_ui->twAgr->item(i,2)->setText(QString("%1").arg(avg,6,'f',2));
	    
	    max=query.value(i*3+3).toDouble() *(m_trinfo->fScale[i][1]-m_trinfo->fScale[i][0])/m_nHeight + m_trinfo->fScale[i][0];
            m_ui->twAgr->item(i,3)->setText(QString("%1").arg(max,6,'f',2));
	}
	query.clear();

    }
    else
    {
	//qDebug() << query.lastError();
    }
	
    //qDebug() << "End";
}
else
{
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    QMessageBox::critical(this,tr("Помилка"),dbs.lastError().databaseText());
}

}

void TrendWindow::colorChange()
{
    int i=0;
    QColor color;
    if(sender())
    {
	i=sender()->objectName().mid(3,1).toInt();	
	color=QColorDialog::getColor(m_Color[i]);
	if(color.isValid())
	{
	    QPalette pal;
	    m_Color[i]=color;

	    pal.setColor(QPalette::Button,m_Color[i]);
	    ((QPushButton*)sender())->setPalette(pal);

	    QPalette pl;
	    pl.setColor(QPalette::Highlight,m_Color[i]);
            m_ui->cursorVal->setPalette(pl);
	
	    m_tw->setColors(m_Color,m_trinfo->numPlot);
	    dataChange();
	}
    }
}

void TrendWindow::plotChange()
{
    // переприв'язка курсора до нової кривої
    int i= sender()->objectName().mid(3,1).toInt();
    QPalette pal;
    pal.setColor(QPalette::Highlight,m_Color[i]);
    m_ui->cursorVal->setPalette(pal);
//    cursorVal->setRange(m_trinfo->fScale[i][0],m_trinfo->fScale[i][1]);// недуже точно але....
    setCursor(-1);    
    
}

void TrendWindow::setCursor(int v)
{
    int i,step;
    unsigned int pos;
    static int ov=0;
    
    if(v==-1)
	v=ov;
    else
	ov=v;

QSqlDatabase dbs=QSqlDatabase::database("history");
if(dbs.isOpen())
{
    
    pos=m_start.toTime_t()+ MyConst::tmr[m_ui->Interval->currentIndex()]*v/m_tw->width();

    m_cursor.setTime_t(pos);    
    m_ui->cursorDate->setText(m_cursor.toString("hh:mm:ss\ndd:MM:yy"));
    
    if(m_nLen > 1)
    {
	step=i=m_nLen/2; // пошук найближчого значення. метод ловлі лева в пуслелі (метод дихотомії)
	do
	{	
	    step>>=1;
	    if(m_pnDt[i]>pos)
		i-=step;
	    else
		i+=step;		
	}while (step>0);
	step=i; // зберегти значення
	
	QSqlQuery qry(dbs);
	QString s;

        for(i=0;i<m_trinfo->numPlot;++i)
        {
	    s+=",";
    	    s+=pv[i]->checkState()==Qt::Checked?m_trinfo->fields[i]:"-1";
	}
	
	if(qry.exec(QString("SELECT Dt%1 from %2 WHERE Dt= %3").arg(s).arg(m_trinfo->table).arg(m_pnDt[step])))
	{
	    qry.next();	    
	    double val;	    
	    for(i=0;i<m_trinfo->numPlot;++i)
	    {
                //qDebug() << i+1 << qry.value(i+1).toDouble() << m_trinfo->fScale[i][1] << m_trinfo->fScale[i][0] << m_nHeight;

                val=(qry.value(i+1).toDouble()*	(m_trinfo->fScale[i][1]-m_trinfo->fScale[i][0]))/(double)m_nHeight + m_trinfo->fScale[i][0];
                m_ui->twAgr->item(i,0)->setText(QString("%1").arg(val,6,'f',2));
		if(ps[i]->isChecked())
		{
                    m_ui->cursorVal->setValue(qry.value(i+1).toInt());
                    m_ui->cursorLCD->display(fabs(val)<1.5?ceil(val):val);

		}
	    }
	    qry.clear();
	}
	else
	{
	    //qDebug() << qry.lastError();
	}
    }
}
else
{
    QMessageBox::critical(this,tr("Помилка"),dbs.lastError().driverText());
}

}

//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------

// Функції-члени класу TrendView
TrendView::TrendView(QWidget *p) : QWidget(p),
    m_nH(4000)
{
    m_px=new QPixmap(2000,2000);
    m_px->fill(QColor(Qt::black));

    m_paint.begin(m_px);
    
    repaint();
    m_bStart = false;
    m_nPlot=0; // на всяк пожарний 
}

TrendView::~TrendView()
{
    m_paint.end();
    delete m_px;
}

void TrendView::start(int nLen,int numPlot,int nHeight /*=4000*/) // тут створ.ється новий oб'єкт m_px
{
    if(m_paint.isActive())
        m_paint.end();    
    
    if(m_px)
        delete m_px; // знищити старий малюнок

    m_px = new QPixmap(size());  // створити новий малюнок
    if(m_px)
    {
	m_paint.begin(m_px);
	m_paint.scale((double)m_px->width()/(double)nLen,(double)m_px->height()/(double)nHeight);

        m_px->fill(Qt::black);     // замалювати фон
        m_nPlot=numPlot;
        m_bStart=true;
    
        m_nH=nHeight; // зберегти висоту графіку
    }
    else
    {
	QMessageBox::critical(this,tr("Помилка"),tr("Не вдалося виділити пам\'ять під об\'єкт QPixmap"));
	close();
    }
}

void TrendView::setData(double nX,QVector<int>& Data) // цей слот приймає дані для малювання.
{
    int i;

    if(m_bStart) // якщо рисуємо першу точку
    {
	m_bStart= false	;
    }
    else
    {
	for(i=0;i<m_nPlot;++i)
	{
	    m_pen.setColor(m_pColor[i]);
	    m_paint.setPen(m_pen);
	    m_paint.drawLine(m_nX,m_pfData[i],nX,m_nH-Data[i]); // намалювати поточний відрізок	    
	}
    }
    // зберегти стару точну
        for(i=0;i<m_nPlot;++i)
	{
		m_pfData[i]=m_nH-Data[i];
        }
        m_nX=nX;    
}

void TrendView::draw() // власне по цій команді об'єкт m_px відображається на екрані
{
    repaint(); // а можливо це непотрібно ?
}

void TrendView::setColors(QColor *pColor,int numPlot)
{
    int i;
    for(i=0;i<numPlot;++i)
    {
	m_pColor[i]=pColor[i];	
    }
}

void TrendView::paintEvent(QPaintEvent *) // малюю вищенаведену картину на екрані.
{
    QPainter pnt(this);
    QPen p(QColor(Qt::yellow));
    
    pnt.drawPixmap(0,0,*m_px);

    pnt.setPen(p);
    pnt.drawLine(m_nCursorPos,0,m_nCursorPos,height()); // намалювати курсор
    
}

/* схоче що це поки-що непортрібно, управлінням розміром об'єкту керує менеджер розміщення */
void TrendView::resizeEvent(QResizeEvent *e)
{    
//    resize(e->size());
    if(e->size()!=e->oldSize()) // якщо розміри змінились, сказати що треба перемалюватися
        emit _redraw();
} 

void TrendView::mouseMoveEvent(QMouseEvent *e)
{    
    if (e->buttons()&Qt::LeftButton)
    {
        m_nCursorPos=e->x();
	emit cursorMoved(m_nCursorPos);
        repaint();
    }
}

void TrendView::mousePressEvent(QMouseEvent *e)
{
    if (e->button()==Qt::LeftButton)
    {
        m_nCursorPos=e->x();
	emit cursorMoved(m_nCursorPos);
        repaint();
    }    
}	

