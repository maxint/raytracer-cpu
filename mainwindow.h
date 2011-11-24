#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QMenu;
class QLabel;
class QAction;
class QActionGroup;
class QDoubleSpinBox;
class QProgressBar;

namespace trimeshVec {
	class CAccessObj;
}

namespace RayTracer {
	class Engine;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	MainWindow(const QString &fileName);
	~MainWindow();

protected:
	 bool eventFilter(QObject *obj, QEvent *e);
	 QSize sizeHint() const;

private:
	void setupUi();
	void createActions();
	void createMenus();
	void createToolBars();
	void createStatusBar();
	void init();
	void initRenderSystem();
	void openObjFile(const QString& fileName);
	void renderObj();
	void saveAsImageFile(const QString& fileName);
	void setResolution(int width, int height);
	QString strippedName(const QString& fullFileName);
	void rotateBy(double xAngle, double yAngle, double zAngle);
	void updateInformationBar();

private slots:
	void open();
	void saveAs();
	void resolution();
	void shadeModel(QAction* act);
	void toggleView(QAction* act);
	void newFrustumOrLight();
	void about();

private:
	QWidget *mImgView;
	QImage mImage;
	QMenu *mFileMenu;
	QMenu *mViewMenu;
	QMenu *mEditMenu;
	QMenu *mHelpMenu;
	QToolBar *mFileToolBar;
	QToolBar *mEditToolBar;
	QToolBar *mCameraLightToolBar;
	QAction *mOpenAct;
	QAction *mQuitAct;
	QAction *mSaveAsImageAct;
	QAction *mResolutionAct;
	QActionGroup *mShadeActGroup;
	QAction *mRenderAct;
	QActionGroup *mViewActGroup;
	QAction *mViewToolBarAct;
	QAction *mInfoToolBarAct;
	QAction *mAboutAct;
	QAction *mAboutQtAct;

	QDoubleSpinBox *mSpinEyeX;
	QDoubleSpinBox *mSpinEyeY;
	QDoubleSpinBox *mSpinEyeZ;
	QDoubleSpinBox *mSpinLightX;
	QDoubleSpinBox *mSpinLightY;
	QDoubleSpinBox *mSpinLightZ;

	// info toolbar
	QToolBar *mInfoToolBar;
	QLabel *mInfoLabel;
	long mLastCostTime;
	QAction *mTraceDepthAct;
	QAction *mRegularSamplesAct;

	// status bar
	QLabel *mResLabel;
	QProgressBar *mProgressBar;

	trimeshVec::CAccessObj *mpAccessObj;
	RayTracer::Engine *mEngine;

	// mouse operations
	QPoint lastPos;
	int xRot;
	int yRot;
	int zRot;
};

#endif // MAINWINDOW_H
