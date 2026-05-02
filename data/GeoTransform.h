#pragma once
#include <QPointF>
#include <QTransform>
#include <cmath>

/// @brief 逻辑坐标（米）到屏幕坐标（像素）的仿射变换。
///
/// 设计思路：
///   逻辑坐标系：以校园实际尺寸为基准，单位为米。
///     - 原点在校园西南角
///     - x 轴向东，y 轴向北
///   屏幕坐标系：QGraphicsScene 坐标，单位为像素。
///     - 原点在左上角，x 向右，y 向下
///
/// 变换链：逻辑坐标 → 缩放 → 等轴测旋转 → 平移到场景中心
///
class GeoTransform {
public:
    GeoTransform() = default;

    /// 构造变换矩阵
    /// @param metersPerPixel  比例尺（每像素对应多少米），如 0.5 表示 1px = 0.5m
    /// @param originScreen   逻辑原点在屏幕上的位置（像素）
    void configure(double metersPerPixel, QPointF originScreen) {
        m_mpp = metersPerPixel;
        m_origin = originScreen;

        // 等轴测矩阵（2:1 比率，30° 角）
        // 逻辑 x → 屏幕右下方
        // 逻辑 y → 屏幕左下方
        double sx = 1.0 / m_mpp;  // 逻辑米 → 像素比例
        double sy = sx;

        // 等轴测基向量：
        //   屏幕右 = (cos30°, sin30°) = (0.866, 0.5)
        //   屏幕上 = (cos30°,-sin30°) = (0.866,-0.5)
        // 因为 QGraphicsScreen y 向下，所以"北"映射到屏幕上方（y 负方向）
        const double c30 = std::cos(30.0 * M_PI / 180.0);  // 0.866
        const double s30 = std::sin(30.0 * M_PI / 180.0);  // 0.5

        // 仿射矩阵：
        //   sx_screen = (logicX + logicY) * c30 * sx + originX
        //   sy_screen = (logicX - logicY) * s30 * sy + originY
        //
        // 展开为 QTransform 的 3×2 矩阵：
        //   [ m11  m12  m13 ]   [ a  b  tx ]
        //   [ m21  m22  m23 ] = [ c  d  ty ]
        //   [  0    0    1  ]   [ 0  0   1 ]
        //
        m_matrix = QTransform(
            c30 * sx,   s30 * sx,    // 第一行：x_screen = a*lx + b*ly + tx
           -c30 * sy,   s30 * sy,    // 第二行：y_screen = c*lx + d*ly + ty
            m_origin.x(), m_origin.y()  // 平移
        );

        m_inverted = m_matrix.inverted();
    }

    /// 逻辑坐标 → 屏幕坐标
    QPointF toScreen(double logicX, double logicY) const {
        return m_matrix.map(QPointF(logicX, logicY));
    }

    /// 屏幕坐标 → 逻辑坐标（用于鼠标点击反算）
    QPointF toLogic(double screenX, double screenY) const {
        return m_inverted.map(QPointF(screenX, screenY));
    }

    /// 获取比例尺
    double metersPerPixel() const { return m_mpp; }

    /// 获取变换矩阵（用于 QPainter::setTransform 等）
    const QTransform& matrix() const { return m_matrix; }

private:
    double m_mpp = 1.0;           // 比例尺：米/像素
    QPointF m_origin = {0, 0};    // 逻辑原点的屏幕位置
    QTransform m_matrix;          // 正向变换
    QTransform m_inverted;        // 逆变换
};
