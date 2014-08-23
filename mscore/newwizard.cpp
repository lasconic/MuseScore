//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: newwizard.cpp 5626 2012-05-13 18:33:52Z lasconic $
//
//  Copyright (C) 2008 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "newwizard.h"
#include "musescore.h"
#include "preferences.h"
#include "palette.h"
#include "instrdialog.h"

#include "libmscore/instrtemplate.h"
#include "libmscore/score.h"
#include "libmscore/staff.h"
#include "libmscore/clef.h"
#include "libmscore/part.h"
#include "libmscore/drumset.h"
#include "libmscore/keysig.h"
#include "libmscore/measure.h"
#include "libmscore/stafftype.h"
#include "libmscore/timesig.h"
#include "libmscore/sym.h"

namespace Ms {

extern Palette* newKeySigPalette();
extern void filterInstruments(QTreeWidget *instrumentList, const QString &searchPhrase = QString(""));

//---------------------------------------------------------
//   TimesigWizard
//---------------------------------------------------------

TimesigWizard::TimesigWizard(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      connect(tsCommonTime, SIGNAL(toggled(bool)), SLOT(commonTimeToggled(bool)));
      connect(tsCutTime,    SIGNAL(toggled(bool)), SLOT(cutTimeToggled(bool)));
      connect(tsFraction,   SIGNAL(toggled(bool)), SLOT(fractionToggled(bool)));
      }

//---------------------------------------------------------
//   measures
//---------------------------------------------------------

int TimesigWizard::measures() const
      {
      return measureCount->value();
      }

//---------------------------------------------------------
//   timesig
//---------------------------------------------------------

Fraction TimesigWizard::timesig() const
      {
      if (tsFraction->isChecked())
            return Fraction(timesigZ->value(), 1 << timesigN->currentIndex());
      else if (tsCommonTime->isChecked())
            return Fraction(4, 4);
      else
            return Fraction(2, 2);
      }

//---------------------------------------------------------
//   pickupMeasure
//---------------------------------------------------------

bool TimesigWizard::pickup(int* z, int* n) const
      {
      *z = pickupTimesigZ->value();
      *n = 1 << pickupTimesigN->currentIndex();
      return pickupMeasure->isChecked();
      }

//---------------------------------------------------------
//   type
//---------------------------------------------------------

TimeSigType TimesigWizard::type() const
      {
      if (tsFraction->isChecked())
            return TimeSigType::NORMAL;
      if (tsCommonTime->isChecked())
            return TimeSigType::FOUR_FOUR;
      return TimeSigType::ALLA_BREVE;
      }

//---------------------------------------------------------
//   commonTimeToggled
//---------------------------------------------------------

void TimesigWizard::commonTimeToggled(bool val)
      {
      if (val) {
            // timesigZ->setValue(4);
            // timesigN->setValue(4);
            timesigZ->setEnabled(false);
            timesigN->setEnabled(false);
            }
      }

//---------------------------------------------------------
//   cutTimeToggled
//---------------------------------------------------------

void TimesigWizard::cutTimeToggled(bool val)
      {
      if (val) {
            // timesigZ->setValue(2);
            // timesigN->setValue(2);
            timesigZ->setEnabled(false);
            timesigN->setEnabled(false);
            }
      }

//---------------------------------------------------------
//   fractionToggled
//---------------------------------------------------------

void TimesigWizard::fractionToggled(bool val)
      {
      if (val) {
            timesigZ->setEnabled(true);
            timesigN->setEnabled(true);
            }
      }

//---------------------------------------------------------
//   TitleWizard
//---------------------------------------------------------

TitleWizard::TitleWizard(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      }

//---------------------------------------------------------
//   NewWizardPage1
//---------------------------------------------------------

NewWizardPage1::NewWizardPage1(QWidget* parent)
   : QWizardPage(parent)
      {
      setTitle(tr("Create New Score"));
      setSubTitle(tr("This wizard creates a new score"));
      //crash setAccessibleName(title());
      setAccessibleName(tr("Create New Score"));
      setAccessibleDescription(subTitle());

      w = new TitleWizard;

      registerField("useTemplate", w->rb1, "checked");
      QGridLayout* grid = new QGridLayout;
      grid->addWidget(w, 0, 0);
      setLayout(grid);
      }

//---------------------------------------------------------
//   initializePage
//---------------------------------------------------------

void NewWizardPage1::initializePage()
      {
      w->title->setText("");
      w->subtitle->setText("");
      // w->composer->text();
      // w->poet->text();
      // w->copyright->text();
      }

//---------------------------------------------------------
//   NewWizardPage2
//---------------------------------------------------------

NewWizardPage2::NewWizardPage2(QWidget* parent)
   : QWizardPage(parent)
      {
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Define a set of instruments. Each instrument"
                     " is represented by one or more staves"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());
      complete = false;
      w = new InstrumentsWidget;
      QGridLayout* grid = new QGridLayout;
      grid->setSpacing(0);
      grid->setContentsMargins(0, 0, 0, 0);
      grid->addWidget(w, 0, 0);
      setLayout(grid);
      connect(w, SIGNAL(completeChanged(bool)), this, SLOT(setComplete(bool)));
      }

//---------------------------------------------------------
//   initializePage
//---------------------------------------------------------

void NewWizardPage2::initializePage()
      {
      w->init();
      }

//---------------------------------------------------------
//   setComplete
//---------------------------------------------------------

void NewWizardPage2::setComplete(bool val)
      {
      complete = val;
      emit completeChanged();
      }

//---------------------------------------------------------
//   createInstruments
//---------------------------------------------------------

void NewWizardPage2::createInstruments(Score* s)
      {
      w->createInstruments(s);
      }

//---------------------------------------------------------
//   NewWizardPage3
//---------------------------------------------------------

NewWizardPage3::NewWizardPage3(QWidget* parent)
   : QWizardPage(parent)
      {
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Create Time Signature"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());

      w = new TimesigWizard;
      QGridLayout* grid = new QGridLayout;
      grid->addWidget(w, 0, 0);
      setLayout(grid);
      }

class GridScene : public QGraphicsScene
{
public:
    GridScene()
    {
    }
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
    {
        for (QGraphicsItem* item : items()) {
            QGraphicsRectItem* rectItem = qgraphicsitem_cast<QGraphicsRectItem*>(item);
            if (rectItem)
                rectItem->setPen(QPen(Qt::transparent));
        }
        QGraphicsRectItem* mDragged = qgraphicsitem_cast<QGraphicsRectItem*>(itemAt(mouseEvent->scenePos(), QTransform()));
        if (mDragged)
            mDragged->setPen(QPen(Qt::blue));
    }
};

//---------------------------------------------------------
//   NewWizardPage4
//---------------------------------------------------------

NewWizardPage4::NewWizardPage4(QWidget* parent)
   : QWizardPage(parent)
      {
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Select Template File:"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());

      /*templateFileDialog = new QFileDialog;
      templateFileDialog->setParent(this);
      templateFileDialog->setModal(false);
      templateFileDialog->setSizeGripEnabled(false);
      templateFileDialog->setFileMode(QFileDialog::ExistingFile);
      templateFileDialog->setOption(QFileDialog::DontUseNativeDialog, true);
      templateFileDialog->setWindowTitle(tr("MuseScore: Select Template"));
      QString filter = tr("MuseScore Template Files (*.mscz *.mscx)");
      templateFileDialog->setNameFilter(filter);
      templateFileDialog->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored));

      QFileInfo myTemplates(preferences.myTemplatesPath);
      if (myTemplates.isRelative())
            myTemplates.setFile(QDir::home(), preferences.myTemplatesPath);
      QList<QUrl> urls;
      urls.append(QUrl::fromLocalFile(mscoreGlobalShare + "templates"));
      urls.append(QUrl::fromLocalFile(myTemplates.absoluteFilePath()));
      templateFileDialog->setSidebarUrls(urls);

      QSettings settings;
      templateFileDialog->restoreState(settings.value("templateFileDialog").toByteArray());
      templateFileDialog->setAcceptMode(QFileDialog::AcceptOpen);
      templateFileDialog->setDirectory(mscoreGlobalShare + "templates");*/

      QLayout* layout = new QVBoxLayout;
      view = new QGraphicsView(this);
      //view->setStyleSheet("background-color:black;");
      
      struct MyTemplate {
      	QString path;
            QString imagePath;
            QString name;
            QString category;
            
      MyTemplate(QString t, QString n, QString o, QString o2)
         : path(t), imagePath(n), name(o), category(o2){}
      };
      QMultiMap<QString, MyTemplate> map;
      
      
      QFileInfo myTemplates(preferences.myTemplatesPath);
      if (myTemplates.isRelative())
            myTemplates.setFile(QDir::home(), preferences.myTemplatesPath);
      
      
      QDir d;
      d.mkpath(dataPath + "/templateImageCache");
      
      QDirIterator dirIt(myTemplates.absoluteFilePath(), QDirIterator::Subdirectories);
      while (dirIt.hasNext()) {
            dirIt.next();
            if (QFileInfo(dirIt.filePath()).isFile()) {
            	QString suffix = QFileInfo(dirIt.filePath()).suffix();
                  if (suffix == "mscx" || suffix == "mscz") {
                        QFileInfo fi(dirIt.filePath());
                        QStringList sl = fi.baseName().split("-");
                        if (sl.size() == 3) {
                        	QFileInfo fi = dirIt.fileInfo();
                              QString imgPath = QString("%1/templateImageCache/%2.png").arg(dataPath).arg(fi.baseName());
                              QString category = QString("%1-%2").arg(sl[0]).arg(sl[1]);
                              QString name = sl[2].replace("_", " ");
                        	map.insert(category, MyTemplate(dirIt.filePath(), imgPath, name, category));
                              }
                        }
                  }
		}
      
      QDirIterator dirIt2(mscoreGlobalShare + "templates", QDirIterator::Subdirectories);
      while (dirIt2.hasNext()) {
            dirIt2.next();
            if (QFileInfo(dirIt2.filePath()).isFile()) {
            	QString suffix = QFileInfo(dirIt2.filePath()).suffix();
                  if (suffix == "mscx" || suffix == "mscz") {
                        QFileInfo fi(dirIt2.filePath());
				QStringList sl = fi.baseName().split("-");
                        if (sl.size() == 3) {
                        	QFileInfo fi = dirIt2.fileInfo();
                        	 QString imgPath = QString("%1/templateImageCache/%2.png").arg(dataPath).arg(fi.baseName());
                              QString category = QString("%1-%2").arg(sl[0]).arg(sl[1]);
                              QString name = sl[2].replace("_", " ");
                        	map.insert(category, MyTemplate(dirIt2.filePath(), imgPath, name, category));
                              }
                        }
                  }
		}
      

      QSize cellSize(150, 200);

      view->setScene(new GridScene());
      
      qreal left = int(rect().left()) - (int(rect().left()) % cellSize.width());
      qreal top = int(rect().top()) - (int(rect().top()) % cellSize.height());
	view->resize(size());
      view->setSceneRect(0, 0, size().width(), size().height());

      qreal headerHeight = 17;
      qreal topOffset = headerHeight;
      
      for (QString category : map.uniqueKeys()) {
          QList<MyTemplate> templatePaths = map.values(category);
          QGraphicsTextItem * text = view->scene()->addText(category.split("-")[1]);
          text->setPos(QPointF(0, topOffset - headerHeight - 2));
          QFont font(text->font());
          font.setBold(true);
          text->setFont(font);
          int count = 0;
          int maxCount = templatePaths.size();
          qreal y = 0;
          for (y = top + topOffset; count != maxCount; y += cellSize.height()) {
              for (qreal x = left; x < rect().right() && count != maxCount;  x += cellSize.width()) {
                    MyTemplate tpl = templatePaths.at(count);
                    QFileInfo fi(tpl.path);
                    QGraphicsPixmapItem* gp = new QGraphicsPixmapItem(QPixmap(tpl.imagePath));
                    gp->setOffset(QPointF(x + 15, y + 5));
                    view->scene()->addItem(gp);
                    QGraphicsRectItem* rectItem = view->scene()->addRect(x, y, cellSize.width(), cellSize.height());
                    rectItem->setBrush(QBrush(Qt::transparent));
                    rectItem->setPen(QPen(Qt::transparent));
                    rectItem->setData(0, QVariant(path));
                    QGraphicsTextItem * text = view->scene()->addText(tpl.name);
          		  text->setPos(QPointF(x + text->boundingRect().width()/2, y + cellSize.height() - headerHeight - 2));
                    count++;
                    }
              }
          topOffset = y + headerHeight;
          }

      view->resize(QSize(size().width(), topOffset - headerHeight));
      view->setSceneRect(size().width() % cellSize.width(), 0, size().width(), topOffset - headerHeight);
      view->show();
      
      
      //sa->setWidget(view);
      layout->addWidget(view);
      
      layout->setMargin(0);
      setLayout(layout);

      //connect(templateFileDialog, SIGNAL(currentChanged(const QString&)), SLOT(templateChanged(const QString&)));
      //connect(templateFileDialog, SIGNAL(accepted()), SLOT(fileAccepted()));
      }

//---------------------------------------------------------
//   initializePage
//---------------------------------------------------------

void NewWizardPage4::initializePage()
      {
      // modify dialog
      // possibly this is not portable as we make some assumptions on the
      // implementation of QFileDialog

      /*templateFileDialog->show();
      QList<QPushButton*>widgets = templateFileDialog->findChildren<QPushButton*>();
      foreach(QPushButton* w, widgets) {
            w->setEnabled(false);
            w->setVisible(false);
            }
      path.clear();
      if (templateFileDialog->selectedFiles().size() > 0) {
            path = templateFileDialog->selectedFiles()[0];
            emit completeChanged();
            }*/
      }

//---------------------------------------------------------
//   isComplete
//---------------------------------------------------------

bool NewWizardPage4::isComplete() const
      {
      return !path.isEmpty();
      }

//---------------------------------------------------------
//   fileAccepted
//---------------------------------------------------------

void NewWizardPage4::fileAccepted()
      {
      //templateFileDialog->show();
      view->show();
      wizard()->next();
      }

//---------------------------------------------------------
//   templateChanged
//---------------------------------------------------------

void NewWizardPage4::templateChanged(const QString& s)
      {
      path = s;
      emit completeChanged();
      }

//---------------------------------------------------------
//   templatePath
//---------------------------------------------------------

QString NewWizardPage4::templatePath() const
      {
      bool useTemplate = field("useTemplate").toBool();
      if (useTemplate)
            return path;
      return QString();
      }


//---------------------------------------------------------
//   NewWizardPage5
//---------------------------------------------------------

NewWizardPage5::NewWizardPage5(QWidget* parent)
   : QWizardPage(parent)
      {
      setTitle(tr("Create New Score"));
      setSubTitle(tr("Select Key Signature and Tempo:"));
      setAccessibleName(title());
      setAccessibleDescription(subTitle());

      QGroupBox* b1 = new QGroupBox;
      b1->setTitle(tr("Key Signature"));
      b1->setAccessibleName(title());
      sp = MuseScore::newKeySigPalette();
      sp->setSelectable(true);
      sp->setSelected(14);
      PaletteScrollArea* sa = new PaletteScrollArea(sp);
      QVBoxLayout* l1 = new QVBoxLayout;
      l1->addWidget(sa);
      b1->setLayout(l1);

      tempoGroup = new QGroupBox;
      tempoGroup->setCheckable(true);
      tempoGroup->setChecked(false);
      tempoGroup->setTitle(tr("Tempo"));
      QLabel* bpm = new QLabel;
      bpm->setText(tr("BPM:"));
      _tempo = new QDoubleSpinBox;
      _tempo->setAccessibleName(tr("Beats per minute"));
      _tempo->setRange(20.0, 400.0);
      _tempo->setValue(100.0);
      QHBoxLayout* l2 = new QHBoxLayout;
      l2->addWidget(bpm);
      l2->addWidget(_tempo);
      l2->addStretch(100);
      tempoGroup->setLayout(l2);

      QVBoxLayout* l3 = new QVBoxLayout;
      l3->addWidget(b1);
      l3->addWidget(tempoGroup);
      l3->addStretch(100);
      setLayout(l3);
      }

//---------------------------------------------------------
//   keysig
//---------------------------------------------------------

KeySigEvent NewWizardPage5::keysig() const
      {
      int idx    = sp->getSelectedIdx();
      Element* e = sp->element(idx);
      return static_cast<KeySig*>(e)->keySigEvent();
      }

//---------------------------------------------------------
//   NewWizard
//---------------------------------------------------------

NewWizard::NewWizard(QWidget* parent)
   : QWizard(parent)
      {
      setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
      setWizardStyle(QWizard::ClassicStyle);
      setPixmap(QWizard::LogoPixmap, QPixmap(":/data/mscore.png"));
      setPixmap(QWizard::WatermarkPixmap, QPixmap());
      setWindowTitle(tr("MuseScore: Create New Score"));
      setOption(QWizard::NoCancelButton, false);
      setOption(QWizard::CancelButtonOnLeft, true);
      setOption(QWizard::HaveFinishButtonOnEarlyPages, true);
      setOption(QWizard::HaveNextButtonOnLastPage, true);


      p1 = new NewWizardPage1;
      p2 = new NewWizardPage2;
      p3 = new NewWizardPage3;
      p4 = new NewWizardPage4;
      p5 = new NewWizardPage5;

      setPage(int(Page::Type), p1);
      setPage(int(Page::Instruments), p2);
      setPage(int(Page::Template), p4);
      setPage(int(Page::Timesig), p3);
      setPage(int(Page::Keysig), p5);
      p2->setFinalPage(true);
      p3->setFinalPage(true);
      p4->setFinalPage(true);
      p5->setFinalPage(true);
      resize(840, 560);
      }

//---------------------------------------------------------
//   nextId
//---------------------------------------------------------

int NewWizard::nextId() const
      {
      switch(Page(currentId())) {
            case Page::Type:
                  return useTemplate() ? int(Page::Template) : int(Page::Instruments);
            case Page::Instruments:
                  return int(Page::Keysig);
            case Page::Keysig:
                  return int(Page::Timesig);
            case Page::Template:
                  return int(Page::Keysig);
            case Page::Timesig:
            default:
                  return int(Page::Invalid);
            }
      }

//---------------------------------------------------------
//   useTemplate
//---------------------------------------------------------

bool NewWizard::useTemplate() const
      {
      return field("useTemplate").toBool();
      }

}

