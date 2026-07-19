#ifndef VISUALIZATION_DATA_HPP_
#define VISUALIZATION_DATA_HPP_

// 可视化数据最大路径点数
#define VISUALIZATION_MAX_PATH_POINTS 2000
// 可视化数据最大路径数量
#define VISUALIZATION_MAX_PATHS 10
// 可视化数据最大项目数量（球体、方块、箭头、圆锥）
#define VISUALIZATION_MAX_ITEMS 10000

// 可视化数据最大网格数量
#define VISUALIZATION_MAX_MESHES 5
// 可视化数据最大网格尺寸（每边的点数）
#define VISUALIZATION_MAX_MESH_GRID 150

#include "cpp_types.hpp"

/**
 * @brief 调试用球体可视化结构体
 *
 * 用于在RViz等可视化工具中绘制球体，用于标记特定位置或状态
 */
struct SphereVisualization {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Vec3< float > position;  ///< 球体中心位置 (x, y, z)
    Vec4< float > color;     ///< 球体颜色 (r, g, b, a)，范围0-1
    double        radius;    ///< 球体半径
};

/**
 * @brief 调试用方块可视化结构体
 *
 * 用于在RViz等可视化工具中绘制立方体，用于标记障碍物或边界
 */
struct BlockVisualization {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Vec3< float > dimension;       ///< 方块尺寸 (width, height, depth)
    Vec3< float > corner_position; ///< 方块角点位置（通常是最小角点）
    Vec3< float > rpy;             ///< 方块姿态（欧拉角：roll, pitch, yaw）
    Vec4< float > color;           ///< 方块颜色 (r, g, b, a)，范围0-1
};

/**
 * @brief 调试用箭头可视化结构体
 *
 * 用于在RViz等可视化工具中绘制箭头，用于表示方向、力、速度等矢量
 */
struct ArrowVisualization {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Vec3< float > base_position; ///< 箭头起点位置
    Vec3< float > direction;     ///< 箭头方向向量
    Vec4< float > color;         ///< 箭头颜色 (r, g, b, a)，范围0-1
    float         head_width;    ///< 箭头头部宽度
    float         head_length;   ///< 箭头头部长度
    float         shaft_width;   ///< 箭头杆部宽度
};

/**
 * @brief 调试用机器人可视化结构体
 *
 * 用于在RViz中绘制简化的机器人模型，与当前仿真的机器人类型一致
 * 可用于显示期望姿态或参考轨迹
 */
struct Cyberdog2Visualization {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Vec12< float > q;    ///< 12个关节角度（FR_hip, FR_thigh, FR_calf, FL_hip, FL_thigh, FL_calf, RR_hip, RR_thigh, RR_calf, RL_hip, RL_thigh, RL_calf）
    Quat< float >  quat; ///< 机器人身体四元数姿态 (w, x, y, z)
    Vec3< float >  p;    ///< 机器人身体位置 (x, y, z)
    Vec4< float >  color; ///< 机器人颜色 (r, g, b, a)，范围0-1
};

/**
 * @brief 调试用路径可视化结构体
 *
 * 用于在RViz中绘制路径轨迹，如规划的行走路径、足迹序列等
 */
struct PathVisualization {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    size_t        num_points = 0;                       ///< 当前路径的点数
    Vec4< float > color;                                ///< 路径颜色 (r, g, b, a)，范围0-1
    Vec3< float > position[ VISUALIZATION_MAX_PATH_POINTS ]; ///< 路径点数组

    /**
     * @brief 清空路径点
     */
    void clear() {
        num_points = 0;
    }
};

/**
 * @brief 调试用圆锥可视化结构体
 *
 * 用于在RViz中绘制圆锥体，可用于表示传感器检测范围、旋转方向等
 */
struct ConeVisualization {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Vec3< float > point_position; ///< 圆锥顶点位置
    Vec3< float > direction;      ///< 圆锥轴线方向
    Vec4< float > color;          ///< 圆锥颜色 (r, g, b, a)，范围0-1
    double        radius;         ///< 圆锥底面半径
};

/**
 * @brief 网格高度图可视化结构体
 *
 * 用于在RViz中绘制2D高度图，如地形、脚力分布等
 */
struct MeshVisualization {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    Vec3< float >                                                                    left_corner; ///< 网格左下角位置 (x, y, z)
    Eigen::Matrix< float, VISUALIZATION_MAX_MESH_GRID, VISUALIZATION_MAX_MESH_GRID > height_map;  ///< 高度图矩阵，存储每个网格点的高度值

    int rows, cols;     ///< 实际使用的网格行数和列数
    float grid_size;    ///< 每个网格单元的尺寸（米）
    float height_max;   ///< 高度图的最大高度值
    float height_min;   ///< 高度图的最小高度值
};

/**
 * @brief 所有调试可视化数据的集合
 *
 * 包含球体、方块、箭头、圆锥、路径和网格等所有可视化元素
 * 通过共享内存从控制器传递到可视化工具
 */
struct VisualizationData {
    size_t              num_paths = 0, num_arrows = 0, num_cones = 0, num_spheres = 0, num_blocks = 0, num_meshes = 0; ///< 各类型元素的数量
    SphereVisualization spheres[ VISUALIZATION_MAX_ITEMS ];   ///< 球体数组
    BlockVisualization  blocks[ VISUALIZATION_MAX_ITEMS ];    ///< 方块数组
    ArrowVisualization  arrows[ VISUALIZATION_MAX_ITEMS ];    ///< 箭头数组
    ConeVisualization   cones[ VISUALIZATION_MAX_ITEMS ];     ///< 圆锥数组
    PathVisualization   paths[ VISUALIZATION_MAX_PATHS ];     ///< 路径数组
    MeshVisualization   meshes[ VISUALIZATION_MAX_MESHES ];   ///< 网格数组

    /**
     * @brief 清空所有调试数据
     */
    void clear() {
        num_paths = 0, num_arrows = 0, num_cones = 0, num_spheres = 0, num_blocks = 0;
        num_meshes = 0;
    }

    /**
     * @brief 添加一个新球体
     *
     * @return 返回新球体指针，若空间不足则返回nullptr
     */
    SphereVisualization* AddSphere() {
        if ( num_spheres < VISUALIZATION_MAX_ITEMS ) {
            return &spheres[ num_spheres++ ];
        }
        return nullptr;
    }

    /**
     * @brief 添加一个新方块
     *
     * @return 返回新方块指针，若空间不足则返回nullptr
     */
    BlockVisualization* AddBlock() {
        if ( num_blocks < VISUALIZATION_MAX_ITEMS ) {
            return &blocks[ num_blocks++ ];
        }
        return nullptr;
    }

    /**
     * @brief 添加一个新箭头
     *
     * @return 返回新箭头指针，若空间不足则返回nullptr
     */
    ArrowVisualization* AddArrow() {
        if ( num_arrows < VISUALIZATION_MAX_ITEMS ) {
            return &arrows[ num_arrows++ ];
        }
        return nullptr;
    }

    /**
     * @brief 添加一个新圆锥
     *
     * @return 返回新圆锥指针，若空间不足则返回nullptr
     */
    ConeVisualization* AddCone() {
        if ( num_cones < VISUALIZATION_MAX_ITEMS ) {
            return &cones[ num_cones++ ];
        }
        return nullptr;
    }

    /**
     * @brief 添加一条新路径
     *
     * @return 返回新路径指针，若空间不足则返回nullptr
     */
    PathVisualization* AddPath() {
        if ( num_paths < VISUALIZATION_MAX_PATHS ) {
            auto* path = &paths[ num_paths++ ];
            path->clear();
            return path;
        }
        return nullptr;
    }

    /**
     * @brief 添加一个新网格
     *
     * @return 返回新网格指针，若空间不足则返回nullptr
     */
    MeshVisualization* AddMesh() {
        if ( num_paths < VISUALIZATION_MAX_MESHES ) {
            return &meshes[ num_meshes++ ];
        }
        return nullptr;
    }
};

#endif  // VISUALIZATION_DATA_HPP_
