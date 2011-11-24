#include "mainwindow.h"
#include "AccessObj.h"
#include <QtGui>
#include <ctime>
#include "raytracer.h"

using namespace RayTracer;

const QString WIDGET_NAME = "ImageView";
const QString WINDOW_TITLE = "Ray Tracer CPU";
const RayTracer::Vec3 EYE_POS(3, 4, 5);
const RayTracer::Vec3 LIGHT_POS(2, 3, 4);
const int TOOLTIP_STRETCH = 5000;

MainWindow::MainWindow()
: mImage(800, 600, QImage::Format_RGB888)
, mpAccessObj(0)
, mEngine(0)
, mLastCostTime(0)
{
	init();
}

MainWindow::MainWindow(const QString &fileName)
: mImage(800, 600, QImage::Format_RGB888)
, mpAccessObj(0)
, mEngine(0)
{
	init();
	if (QFile::exists(fileName))
	{
		openObjFile(fileName);
		statusBar()->showMessage(tr("File loaded"), TOOLTIP_STRETCH);
	}
	else
	{
		statusBar()->showMessage(tr("File load fail"));
	}
}

MainWindow::~MainWindow()
{
	SAFE_DELETE(mpAccessObj);
}

void MainWindow::init()
{
	srand((unsigned int)time(0));
	xRot = yRot = zRot = 30;
	
	initRenderSystem();

	setupUi();
	createActions();
	createMenus();
	createToolBars();
	createStatusBar();	
}

void MainWindow::initRenderSystem()
{
	mpAccessObj = new trimeshVec::CAccessObj;
	mEngine = new RayTracer::Engine;
	mEngine->setRenderTarget(mImage.width(), mImage.height(), &mImage);

	mImage.fill(qRgb(200, 200, 200));
}

void MainWindow::setupUi()
{
	mImgView = new QWidget;
	mImgView->setObjectName(WIDGET_NAME);
	mImgView->setFixedSize(mImage.size());
	this->setCentralWidget(mImgView);
	mImgView->installEventFilter(this);
	this->layout()->setSizeConstraint(QLayout::SetFixedSize);

	mProgressBar = new QProgressBar(this);

	setWindowTitle(WINDOW_TITLE);
}

QSize MainWindow::sizeHint() const
{
	return mImage.size();
}

void MainWindow::createActions()
{
	// file menu
	mOpenAct = new QAction(QIcon(":/images/open.png"), tr("&Open Obj..."), this);
	mOpenAct->setShortcut(QKeySequence::Open);
	mOpenAct->setToolTip(tr("Open an obj model file"));
	connect(mOpenAct, SIGNAL(triggered()), this, SLOT(open()));

	mSaveAsImageAct = new QAction(QIcon(":/images/save.png"), tr("&Save As Image..."), this);
	mSaveAsImageAct->setShortcut(QKeySequence::Save);
	mSaveAsImageAct->setToolTip(tr("Save the result as an image"));
	connect(mSaveAsImageAct, SIGNAL(triggered()), this, SLOT(saveAs()));

	mQuitAct = new QAction(tr("&Quit"), this);
	mQuitAct->setShortcut(tr("Ctrl+Q"));
	mQuitAct->setToolTip(tr("Exit the application"));
	connect(mQuitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

	// edit menu
	mResolutionAct = new QAction(tr("Set &Canvas..."), this);
	mResolutionAct->setShortcut(tr("Ctrl+C"));
	mResolutionAct->setToolTip(tr("Set the resolution"));
	connect(mResolutionAct, SIGNAL(triggered()), this, SLOT(resolution()));

	mRenderAct = new QAction(tr("&Render!"), this);
	mRenderAct->setToolTip(tr("Render the scene"));
	mRenderAct->setShortcut(tr("Ctrl+R"));

	mTraceDepthAct = new QAction(tr("Trace &Depth..."), this);
	mTraceDepthAct->setToolTip(tr("Set Ray tracing depth"));
	mTraceDepthAct->setShortcut(tr("Ctrl+D"));

	mRegularSamplesAct = new QAction(tr("Regular &Sampling Size..."), this);
	mRegularSamplesAct->setToolTip(tr("Set regular sampling size of box light"));

	mShadeActGroup = new QActionGroup(this);
	mShadeActGroup->setExclusive(false);
	mShadeActGroup->addAction(mRenderAct);
	mShadeActGroup->addAction(mTraceDepthAct);
	mShadeActGroup->addAction(mRegularSamplesAct);
	connect(mShadeActGroup, SIGNAL(triggered(QAction*)), this, SLOT(shadeModel(QAction*)));

	// view menu
	mViewToolBarAct = new QAction(tr("&Toolbar"), this);
	mViewToolBarAct->setToolTip(tr("Toggle toolbar"));
	mViewToolBarAct->setCheckable(true);
	mViewToolBarAct->setChecked(true);
	mViewToolBarAct->setShortcut(tr("Ctrl+T"));

	mInfoToolBarAct = new QAction(tr("&Information"), this);
	mInfoToolBarAct->setToolTip(tr("Toggle information toolbar"));
	mInfoToolBarAct->setCheckable(true);
	mInfoToolBarAct->setChecked(true);
	mInfoToolBarAct->setShortcut(tr("Ctrl+I"));

	mViewActGroup = new QActionGroup(this);
	mViewActGroup->addAction(mViewToolBarAct);
	mViewActGroup->addAction(mInfoToolBarAct);
	mViewActGroup->setExclusive(false);
	connect(mViewActGroup, SIGNAL(triggered(QAction*)), this, SLOT(toggleView(QAction*)));

	// help menu
	mAboutAct = new QAction(tr("&About"), this);
	mAboutAct->setToolTip(tr("Show the application's About box"));
	connect(mAboutAct, SIGNAL(triggered()), this, SLOT(about()));
	mAboutQtAct = new QAction(tr("About &Qt"), this);
	mAboutQtAct->setToolTip(tr("Show the Qt library's About box"));
	connect(mAboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void MainWindow::shadeModel(QAction* act)
{
	if (act == mRenderAct)
	{
		renderObj();
	}
	else if (act == mTraceDepthAct)
	{
		bool ok;
		int newDepth = QInputDialog::getInt(this, tr("Set Trace Depth"),
			tr("New Depth (1-%1) : ").arg(RT_TRACEDEPTH), mEngine->getTraceDepth(), 
			1, RT_TRACEDEPTH, 1, &ok);

		if (ok)
		{
			mEngine->setTraceDepth(newDepth);
			statusBar()->showMessage(tr("New ray tracing depth (%1) is applied").arg(newDepth), TOOLTIP_STRETCH);
			renderObj();
		}
	}
	else if (act == mRegularSamplesAct)
	{
		bool ok;
		int newSize = QInputDialog::getInt(this, tr("Set Regular Sampling Size"),
			tr("New sampling size (1-%1) : ").arg(RT_REGULAR_SAMPLES), mEngine->getRegularSampleSize(), 
			1, RT_REGULAR_SAMPLES, 1, &ok);

		if (ok)
		{
			mEngine->setRegularSampleSize(newSize);
			statusBar()->showMessage(tr("New sampling size (%1) is applied").arg(newSize), TOOLTIP_STRETCH);
			renderObj();
		}
	}
	updateInformationBar();
}

void MainWindow::toggleView(QAction *act)
{
	bool bShow = act->isChecked();
	if (act == mViewToolBarAct)
	{
		mFileToolBar->setVisible(bShow);
		mEditToolBar->setVisible(bShow);
		mCameraLightToolBar->setVisible(bShow);
	}
	else if (act == mInfoToolBarAct)
	{
		mInfoToolBar->setVisible(bShow);
	}
}

void MainWindow::createMenus()
{
	mFileMenu = menuBar()->addMenu(tr("&File"));
	mFileMenu->addAction(mOpenAct);
	mFileMenu->addAction(mSaveAsImageAct);
	mFileMenu->addSeparator();
	mFileMenu->addAction(mQuitAct);
	
	menuBar()->addSeparator();

	mViewMenu = menuBar()->addMenu(tr("&View"));
	mViewMenu->addAction(mViewToolBarAct);
	mViewMenu->addAction(mInfoToolBarAct);

	menuBar()->addSeparator();

	mEditMenu = menuBar()->addMenu(tr("&Edit"));
	mEditMenu->addAction(mResolutionAct);
	mEditMenu->addAction(mRenderAct);
	mEditMenu->addAction(mTraceDepthAct);
	mEditMenu->addAction(mRegularSamplesAct);
	mEditMenu->addSeparator();

	menuBar()->addSeparator();
	mHelpMenu = menuBar()->addMenu(tr("&Help"));
	mHelpMenu->addAction(mAboutAct);
	mHelpMenu->addAction(mAboutQtAct);
}

void MainWindow::createToolBars()
{
	// file toolbar
	mFileToolBar = addToolBar(tr("File"));
	mFileToolBar->addAction(mOpenAct);
	mFileToolBar->addAction(mSaveAsImageAct);

	// edit toolbar
	mEditToolBar = addToolBar(tr("Edit"));
	mEditToolBar->addAction(mResolutionAct);
	mEditToolBar->addAction(mRenderAct);
	mEditToolBar->addAction(mTraceDepthAct);
	mEditToolBar->addSeparator();

	// camera and light toolbar
	//mSpinEyeX = new QDoubleSpinBox;
	//mSpinEyeY = new QDoubleSpinBox;
	//mSpinEyeZ = new QDoubleSpinBox;
	//mSpinLightX = new QDoubleSpinBox;
	//mSpinLightY = new QDoubleSpinBox;
	//mSpinLightZ = new QDoubleSpinBox;
	//mSpinEyeX->setMinimum(-100.0);
	//mSpinEyeY->setMinimum(-100.0);
	//mSpinEyeZ->setMinimum(-100.0);
	//mSpinLightX->setMinimum(-100.0);
	//mSpinLightY->setMinimum(-100.0);
	//mSpinLightX->setMinimum(-100.0);
	//mSpinEyeX->setSingleStep(0.2);
	//mSpinEyeY->setSingleStep(0.2);
	//mSpinEyeZ->setSingleStep(0.2);
	//mSpinLightX->setSingleStep(0.2);
	//mSpinLightY->setSingleStep(0.2);
	//mSpinLightZ->setSingleStep(0.2);
	//mSpinEyeX->setValue(EYE_POS.x);
	//mSpinEyeY->setValue(EYE_POS.y);
	//mSpinEyeZ->setValue(EYE_POS.z);
	//mSpinLightX->setValue(LIGHT_POS.x);
	//mSpinLightY->setValue(LIGHT_POS.y);
	//mSpinLightZ->setValue(LIGHT_POS.z);

	//mCameraLightToolBar = addToolBar(tr("CameraAndLight"));
	//addToolBar(Qt::BottomToolBarArea, mCameraLightToolBar);
	//mCameraLightToolBar->setToolTip(tr("Set position of camera or light"));

	//mCameraLightToolBar->addWidget(new QLabel(tr("Camera ")));
	//mCameraLightToolBar->addWidget(new QLabel(tr("x:")));
	//mCameraLightToolBar->addWidget(mSpinEyeX);
	//mCameraLightToolBar->addWidget(new QLabel(tr("y:")));
	//mCameraLightToolBar->addWidget(mSpinEyeY);
	//mCameraLightToolBar->addWidget(new QLabel(tr("z:")));
	//mCameraLightToolBar->addWidget(mSpinEyeZ);
	//mCameraLightToolBar->addSeparator();
	//mCameraLightToolBar->addWidget(new QLabel(tr("Light ")));
	//mCameraLightToolBar->addWidget(new QLabel(tr("x:")));
	//mCameraLightToolBar->addWidget(mSpinLightX);
	//mCameraLightToolBar->addWidget(new QLabel(tr("y:")));
	//mCameraLightToolBar->addWidget(mSpinLightY);
	//mCameraLightToolBar->addWidget(new QLabel(tr("z:")));
	//mCameraLightToolBar->addWidget(mSpinLightZ);

	//connect(mSpinEyeX, SIGNAL(valueChanged(double)), this, SLOT(newFrustumOrLight()));
	//connect(mSpinEyeY, SIGNAL(valueChanged(double)), this, SLOT(newFrustumOrLight()));
	//connect(mSpinEyeZ, SIGNAL(valueChanged(double)), this, SLOT(newFrustumOrLight()));
	//connect(mSpinLightX, SIGNAL(valueChanged(double)), this, SLOT(newFrustumOrLight()));
	//connect(mSpinLightY, SIGNAL(valueChanged(double)), this, SLOT(newFrustumOrLight()));
	//connect(mSpinLightZ, SIGNAL(valueChanged(double)), this, SLOT(newFrustumOrLight()));

	// information toolbar
	mInfoToolBar = addToolBar(tr("Information"));
	addToolBar(Qt::RightToolBarArea, mInfoToolBar);

	QGroupBox *infoGroup = new QGroupBox(tr("Information"));
	QProgressBar *progressBarCopy = new QProgressBar(mProgressBar);
	connect(mProgressBar, SIGNAL(valueChanged(int)), progressBarCopy, SLOT(setValue(int)));
	QBoxLayout *infoLayout = new QBoxLayout(QBoxLayout::TopToBottom);
	mInfoLabel = new QLabel("");
	infoLayout->addWidget(mInfoLabel);
	infoLayout->addWidget(progressBarCopy);
	infoGroup->setLayout(infoLayout);
	mInfoToolBar->addWidget(infoGroup);

	updateInformationBar();
}

void MainWindow::createStatusBar()
{
	mResLabel = new QLabel(this);
	statusBar()->addPermanentWidget(mResLabel);
	mResLabel->setText(tr("%1x%2").arg(mImage.width()).arg(mImage.height()));

	statusBar()->addWidget(mProgressBar);
	mProgressBar->hide();
	
	statusBar()->showMessage(tr("Ready"));
	statusBar()->setSizeGripEnabled(false);
}

void MainWindow::open()
{
	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Open Obj Model"), "objs", tr("Obj Files (*.obj)"));
	if (!fileName.isEmpty())
	{
		openObjFile(fileName);
	}
}

void MainWindow::saveAs()
{
	QString fileName = QFileDialog::getSaveFileName(this, 
		tr("Save Result"), "res", tr("Images (*.png *.jpg *.bmp)"));
	
	saveAsImageFile(fileName);
}

void MainWindow::resolution()
{
	bool ok;
	QString strSize = QInputDialog::getText(this, tr("Set Resolution"),
		tr("New Resolution (e.g. 800 600) : "), QLineEdit::Normal,
		tr("1024 768"), &ok);

	int w = strSize.section(' ', 0, 0).toInt();
	int h = strSize.section(' ', 1, 1).toInt();
	if (ok && w>0 && h>0)
	{
		mImgView->setFixedSize(w, h);
		setResolution(w, h);
		statusBar()->showMessage(tr("New resolution (%1, %2) is applied").arg(w).arg(h), TOOLTIP_STRETCH);
	}
	else if(ok)
	{
		statusBar()->showMessage(tr("Invalid resolution"), TOOLTIP_STRETCH);
	}
}

void MainWindow::openObjFile(const QString& fileName)
{
	if (mpAccessObj->LoadOBJ(fileName.toStdString().c_str()))
	{
		mpAccessObj->UnifiedModel();

		mEngine->loadObjModel(mpAccessObj);

		renderObj();

		setWindowTitle( tr("%1 - %2")
			.arg(strippedName(fileName))
			.arg(WINDOW_TITLE) );
	}
	else
	{
		QMessageBox::warning(this, tr("OBJ File Invalid"), 
			tr("File %1 is not a valid obj model file.").arg(fileName));
	}
}

void MainWindow::renderObj()
{
	updateInformationBar();
	mProgressBar->reset();
	mProgressBar->show();

	qApp->processEvents();

	mEngine->initEngine(Vec3(0, -2, 4), Vec3(0, -2, 0));
	clock_t tt = clock();
	while(!mEngine->render())
	{
		mImgView->update();
		mProgressBar->setValue(mEngine->getCurrProgree());
		qApp->processEvents();
	}

	mProgressBar->setValue(100);
	mProgressBar->hide();
	mLastCostTime = static_cast<long>(clock()-tt);
	statusBar()->showMessage(tr("Ray Tracing finished in %1 ms with %2 primitives.")
		.arg(mLastCostTime).arg(mEngine->getNumOfPrimitives()), TOOLTIP_STRETCH);
	
	updateInformationBar();

	mImgView->update();
}

void MainWindow::saveAsImageFile(const QString& fileName)
{
	mImage.save(fileName);
}

void MainWindow::setResolution(int width, int height)
{
	mResLabel->setText(tr("%1x%2").arg(width).arg(height));
	mImage = mImage.scaled(width, height);
	mEngine->setRenderTarget(mImage.width(), mImage.height(), &mImage);
	renderObj();
}

void MainWindow::newFrustumOrLight()
{
	renderObj();
}

void MainWindow::about()
{
	QMessageBox::about(this, tr("About Ray Tracer CPU"),
		tr("<p>A <b>RayTracer</b> project writed by maxint, in April, 2010.</p>"
		"<p>Email: <a href='mailto:lnychina@gmail.com'>lnychina@gmail.com</a></p>"
		"<p>Blog: <a href='http://hi.baidu.com/maxint'>http://hi.baidu.com/maxint</a></p>"));
}

void MainWindow::rotateBy(double xAngle, double yAngle, double zAngle)
{
	//static Mat22d matRot;
	//static Vec3 eyePos;

	//xRot += xAngle;
	//yRot += yAngle;
	//zRot += zAngle;
	//xRot %= 360;
	//yRot %= 360;
	//zRot %= 360;

	//eyePos[0] = mSpinEyeX->value();
	//eyePos[1] = mSpinEyeY->value();
	//eyePos[2] = mSpinEyeZ->value();
	//double len = eyePos.len();
	//eyePos[0] = 0;
	//eyePos[1] = 0;
	//eyePos[2] = len;

	//matRot.rotate(-zRot);
	//matRot.map(eyePos[0], eyePos[1], &eyePos[0], &eyePos[1]);
	//std::cout << eyePos.len() << std::endl;
	//matRot.rotate(yRot);
	//matRot.map(eyePos[0], eyePos[2], &eyePos[0], &eyePos[2]);
	//std::cout << eyePos.len() << std::endl;
	//matRot.rotate(-xRot);
	//matRot.map(eyePos[1], eyePos[2], &eyePos[1], &eyePos[2]);
	//std::cout << eyePos.len() << std::endl;

	//mSpinEyeX->setValue(eyePos[0]);
	//mSpinEyeY->setValue(eyePos[1]);
	//mSpinEyeZ->setValue(eyePos[2]);

	newFrustumOrLight();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
	if (obj->isWidgetType() &&
		obj->objectName() == WIDGET_NAME)
	{
		QWidget *wid = static_cast<QWidget*>(obj);
		if (e->type() == QEvent::Paint)
		{
			QPainter painter(wid);
			QRect rect(QPoint(0,0), wid->size());
			painter.drawImage(rect, mImage);
			e->accept();
			return true;
		}
	}
	
	return QMainWindow::eventFilter(obj, e);
}

QString MainWindow::strippedName(const QString& fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

void MainWindow::updateInformationBar()
{
	QString info = tr("<style type='text/css'><!--"
		".info, { margin: 5px; }"
		"--></style>");
	info += tr("<table class='info'>");
	info += tr("<tr><td>Primitives: </td><td>%1</td></tr>").arg(mEngine->getNumOfPrimitives());
	info += tr("<tr><td>Resolution: </td><td>%1 x %2</td></tr>")
		.arg(mImage.width()).arg(mImage.height());
	info += tr("<tr><td>Last cost time: </td><td>%1 ms</td></tr>").arg(mLastCostTime);
	info += tr("<tr><td>Trace depth: </td><td>%1</td></tr>").arg(mEngine->getTraceDepth());
	info += tr("<tr><td>Regular Samples: </td><td>%1</td></tr>").arg(mEngine->getRegularSampleSize());
	info += tr("</table>");
	mInfoLabel->setText(info);
}