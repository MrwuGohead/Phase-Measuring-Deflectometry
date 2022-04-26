#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>

#include "algorithm_PMD.h"
#include "fullscreen.h"
#include "cameracontrol.h"



struct PMDCFG {
    std::string workDir;    // 工作目录
    bool debug;             // 是否 debug 状态
    cv::Size img_size;      // 相机图像分辨率
    Matrix3f cameraMatrix;  // 相机内参
    Mat distcoeffs;         // 相机镜头畸变
    MatrixXf camera_rays;   // 相机光线
    Matrix4f CWT;           // 世界坐标系向量转到相机坐标系下的变换矩阵
    MatrixXf refPlane;      // 相机光线与参考平面的交点
    uint16_t period_width;  // 宽度方向条纹周期数
    uint16_t period_height; // 高度方向条纹周期数
    Screen screen;          // 屏幕信息
    std::vector<Mat> patterns;   // 投影的条纹图案
    std::vector<Eigen::MatrixXi> img_pats;  // 拍摄的条纹图像
    uint16_t screen_delay;  // 投影条纹时等待的时间(毫秒)
};


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

  public:
    MainWindow(QWidget* parent = nullptr);
    void test();
    ~MainWindow();

  signals:
    void projPat(const cv::Mat& img,  uint16_t delay);

  private slots:
    void on_load_cfg_Button_clicked();

    // 调用全屏线程投影条纹图案和捕获图像
    void projAndCapImg();

    void on_sys_calib_Button_clicked();

  private:
    Ui::MainWindow* ui;
    PMDCFG PMDcfg;
    QThread* thread_fullScreen;
    FullScreen* fs;
    CameraControl cameraControl;
};



#endif // MAINWINDOW_H
