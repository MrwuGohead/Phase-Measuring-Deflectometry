#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <json.hpp>
#include <opencv2/core/eigen.hpp>
#include <QFileDialog>

using json = nlohmann::json;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}


// 读取 json 配置文件, 相机内参、条纹参数等, 按钮单击
void MainWindow::on_load_cfg_Button_clicked() {
    // 选择，读取 json 格式配置文件
    QString cfgPath = QFileDialog::getOpenFileName(this, tr("打开配置文件"), "D:/Program/3DReconstruct/Phase Measuring Deflectometry/PMD",
                      tr("json files(*.json);;All files(*.*)"));
    std::ifstream is(cfgPath.toStdString());
    json cfg;
    is >> cfg;
    ui->output_Edit->setPlainText("读取配置文件");

    ui->output_Edit->appendPlainText("读取图片尺寸，相机内参，计算相机光线");
    // 图片尺寸，相机内参，相机光线
    {
        std::vector<float> t1 = cfg.at("cameraMatrix");
        MatrixXfR t2(1, t1.size());
        t2.row(0) = VectorXf::Map(&t1[0], t1.size());
        t2.resize(3, 3);
        PMDcfg.cameraMatrix = t2;

        std::vector<float> t3 = cfg.at("image_size");
        PMDcfg.img_size.width = t3[0];
        PMDcfg.img_size.height = t3[1];

        configCameraRay(PMDcfg.cameraMatrix, PMDcfg.img_size, 1.0, PMDcfg.camera_rays);
    }
    ui->output_Edit->appendPlainText("读取成功!");

    // intersection of camera rays and reference plane
    ui->output_Edit->appendPlainText("读取CWT，计算参考平面与相机光线交点");
    {
        std::vector<float> t1 = cfg.at("CWT");
        MatrixXfR t2(1, t1.size());
        t2.row(0) = VectorXf::Map(&t1[0], t1.size());
        t2.resize(4, 4);
        PMDcfg.CWT = t2;
        configRefPlane(PMDcfg.CWT.block(0, 3, 3, 1), PMDcfg.CWT.block(0, 2, 3, 1), PMDcfg.camera_rays, PMDcfg.refPlane);
    }
    ui->output_Edit->appendPlainText("读取成功!");


    test();
}

void MainWindow::test() {
    // 算数测试
    if (false) {
        ArrayXXf a(2, 2);
        MatrixXf b(2, 2);
        ArrayXf c(2);
        a << 1, 2,
        3, 4;
        b << 5, 6,
        7, 8;
        c << 10, 20;
        Eigen::Vector3f d(1, 2, 3);
        Vector3f t(4, 5, 6);
        Matrix3f f(3, 3);
        f.row(0) = d(0) * t.transpose();

        cout << f << endl;
    }

    // camera rays test, 使用camera_rays 的数据，给定像素坐标，求世界坐标与相机标定时的是否一致.(测试通过)
    if (false) {
        Mat distCoeffs = (cv::Mat_<float>(1, 5) << -0.185079, 0.451621, 0.000411, -0.000481, -3.101673);
        Matrix4f extrinsic;
        Eigen::Vector2i point_img(1517, 748);   // col，row
        Vector3f point_W(0, 0.025, 0);
        extrinsic << -0.01655164577240431, 0.9990216629204951, 0.04100926770613657, -0.05811072408646133,
                  0.9831808940391766, 0.008800869557167967, 0.1824222417671788, -0.05914208463991447,
                  0.1818828541082149, 0.04333891681394951, -0.9823646805900768, 0.6099327406613485,
                  0, 0, 0, 1;
        cv::Size s = cv::Size(4024, 3096);

        Vector3f M(0, 0, 0);
        Vector3f V = PMDcfg.camera_rays.col(point_img(1) * s.width + point_img(0));
        Vector3f N(0.041009, 0.182422, -0.98236);
        Vector3f P(-0.0581107, -0.059142, 0.6099327);
        Vector3f O(3);

        float t = ((P(0) - M(0)) * N(0) + (P(1) - M(1)) * N(1) + (P(2) - M(2)) * N(2)) /
                  (V(0) * N(0) + V(1) * N(1) + V(2) * N(2));
        O(0) = M(0) + V(0) * t;
        O(1) = M(1) + V(1) * t;
        O(2) = M(2) + V(2) * t;
        Vector4f p_o;
        p_o << O, 1;
        p_o = extrinsic.inverse() * p_o;
        cout << "real intersection:" << endl << point_W << endl <<
             "compute intersection: " << endl << p_o << endl;

        Vector4f p_w;
        p_w << point_W, 1;
        p_w = extrinsic * p_w;
        cout << "real intersection:" << endl << p_w << endl <<
             "compute intersection: " << endl << O << endl;

    }

    // 参考平面测试，使用refPlane的数据，给定像素坐标，求世界坐标与相机标定时的是否一致.(测试通过)
    if (true) {
        Matrix4f extrinsic;
        Eigen::Vector2i point_img(1517, 748);   // col，row
        Vector4f point_W(0, 0.025, 0, 1);
        cv::Size s = cv::Size(4024, 3096);
        Vector3f V = PMDcfg.refPlane.col(point_img(1) * s.width + point_img(0));
        cout << "标定点: " << endl << PMDcfg.CWT* point_W << endl
             << "refPlane: " << endl << V << endl;
    }
}















