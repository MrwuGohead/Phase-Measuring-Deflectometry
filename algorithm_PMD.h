#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <inc.h>
#include <opencv2/aruco.hpp>

struct Screen {
    float width;    // 屏幕宽度(m)
    float height;  // 屏幕高度(m)
    uint16_t cols;  // 屏幕分辨率列数
    uint16_t rows;  // 屏幕分辨率行数
};

struct ROI {
    uint32_t startRow;
    uint32_t startCol;
    uint32_t blockRows;
    uint32_t blockCols;
};

/*
 * 重要变量用 Matrix，中间变量要是需要时传 Array
 * 能用 row，col，width，height，就不用x，y
 * 能用Qt的功能就用Qt的，如目录
 * 图像用img，用拍摄图像而非照片表示
 * R旋转，TL平移向量，T变换矩阵，CVT将V坐标系的变量变换到C坐标系的变换矩阵
 */

bool save_matrix_as_img(const MatrixXf& m, std::string path);

/*
 * 矩阵数据输出到文件
 * add_index: 是否将第一行或第一列变为索引下标
 * orientation：0，第一行变为索引，否则，第一列变为索引
*/
bool save_matrix_as_txt(const MatrixXf& m, std::string path, bool add_index = false, int orientation = 0);

/*
 * 矩阵保存为 ply 格式的点云文件
 * params: 矩阵, file path
 */
bool save_matrix_as_ply(const MatrixXf& m, std::string path);

/*
 * 将矩阵变为齐次
 */
MatrixXf matrix_to_home(const MatrixXf& origin);

/*
 * Rays from camera, for every pixel, a line cross this pixel and origin of camera coordinate
 * 每个像素拍摄的路径设为一个向量，本函数产生一个矩形平面，为每个像素生成这样一个向量，每个向量用两点定义，第一点为相机原点，
 * 第二点(x,y,z)为相机坐标系下给定z所求得的三维坐标，忽略镜头畸变
 * 参数：相机内参，相机分辨率，给定Z，camera_rays(相机坐标系下z=给定值的 rays from camera)
 * camrea_rays 格式: 3 x (img.h x img.w), 每列格式为(x,y,z)，像素保存顺序为从左往右，从上往下
 */
bool configCameraRay(const Matrix3f& cameraMatrix, const cv::Size& imageSize, float Z, MatrixXf& camera_rays);

/*
 * 计算相机光线与参考平面的交点，参考平面用点法式表达
 * 参数：参考平面点，参考平面法线，rays from camera，保存交点矩阵
 * refPlane 格式：3 x (img.h x img.w), 每列格式为(x,y,z)，像素保存顺序为从左往右，从上往下
 */
bool configRefPlane(const Eigen::Vector3f& plane_point, const Eigen::Vector3f& plane_normal, const MatrixXf& camera_rays, MatrixXf& refPlane);

/*
 * for every screen pixel, compute their position in world coordinate system
 * params: Screen information, rotation of screen relative to world, translation of screen relative to world, matrix save screen pixels postion,
 * screen_pix_pos 格式: 3 x (screen.h x screen.w), 每列格式为(x,y,z)，像素保存顺序为从左往右，从上往下
 */
bool configScreenPixelPos(const Screen& screen, const Matrix4f& WST, MatrixXf& screen_pix_pos);

/*
 * 生成四步相移法条纹图案
 * 参数：图案尺寸，x，y方向周期，保存结果(size = 16，顺序为高频x,y，低频x,y)
 */
bool pattern_gen(std::vector<uint8_t> pat_size, std::vector<uint8_t> period, std::vector<MatrixXf>& pats);

/* four step phase shifting
 * params: images(size=16), result wraped phase map(size=4)
 */
bool phase_shifting(const std::vector<MatrixXf>& imgs, std::vector<MatrixXf>& wraped_ps_maps);

/*
 * two frequency phase unwrapping
 * params: 包裹相位(高频竖条纹，高频横条纹，低频竖条纹，低频横条纹),高频竖条纹周期数，高频横条纹周期数，保存相位结果
 */
bool phase_unwrapping(const vector<MatrixXf>& wraped_ps_maps, uint16_t period_width, uint16_t period_height, vector<MatrixXf>& unwrap_ps_maps);

/*
 * 求屏幕上相位与相机img对应的像素的三维坐标
 * params：相位图，屏幕信息，条纹图宽度方向上周期数，高度方向周期数，屏幕像素位置，
 * 与相机img相位匹配的屏幕像素坐标（格式: 3 x (cam_img.w x cam_img.h), 每列格式为(x,y,z)，像素保存顺序为从左往右，从上往下）
 */
bool screen_camera_phase_match(const vector<MatrixXf>& unwrap_ps_maps, const Screen& screen, uint16_t period_width, uint16_t period_height,
                               const MatrixXf& screen_pix_pos, MatrixXf& screen_camera_phase_match_pos);

/*
 * 斜率计算
 * params: 相机光心坐标，参考平面，屏幕相机相位匹配，保存斜率结果X和Y方向,尺寸为1 x img.size
 */
bool slope_calculate(const Vector3f& camera_world, const MatrixXf& refPlane, const MatrixXf& screen_camera_phase_match_pos,
                     std::vector<MatrixXfR>& slope);

/*
 * 从斜率恢复相位，modal 法，使用 Zernike polynomials
 * params: x方向斜率(M x N)，y方向斜率，保存高度结果，x和y方向的物理尺寸范围(mm), Zernike polynomials 项数
 */
bool modal_reconstruction(const MatrixXf& sx, const MatrixXf& sy, MatrixXfR& Z, const std::vector<float>& range, uint32_t terms);

double computeReprojectionErrors(
    const vector<vector<cv::Point3f>>& objectPoints,
    const vector<vector<cv::Point2f>>& imagePoints,
    const vector<Mat>& rvecs, const vector<Mat>& tvecs,
    const Mat& cameraMatrix, const Mat& distCoeffs,
    vector<float>& perViewErrors);

// aruco 标定板参数
struct ArucoBoard {
    cv::Size markers_size;
    float markerLength;    // unit: m
    float markerSeparation;
    cv::Ptr<cv::aruco::Dictionary> dictionary;
    float screenOffset_X;
    float screenOffset_Y;
};


bool aruco_analyze(const vector<Mat>& imgs_board, const ArucoBoard& board, const Mat& cameraMatrix, const Mat& distCoeffs, vector<Mat>& Rmats,
                   vector<cv::Vec3f>& Tvecs, vector<vector<int>>& Ids, vector<vector<vector<cv::Point2f>>>& corners,
                   vector<vector<vector<cv::Point2f>>>& rejectedCandidates, std::string output_dir);

/*
 * system geometry calibration
 * implement of paper "Flexible geometrical calibration for fringe-reflection 3D measurement(2012)"
 */
bool system_calib(vector<Matrix3f>& CVR, vector<Vector3f>& CVTL, Matrix3f& CSR, VectorXf& CSTL_D, vector<Vector3f>& n);



#endif // ALGORITHM_H


















