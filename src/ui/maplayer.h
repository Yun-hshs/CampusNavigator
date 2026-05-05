#ifndef MAPLAYER_H
#define MAPLAYER_H

// Z-Value layer management for scene elements.
// Higher values render on top.
enum class MapLayer : int {
    Background    = 0,    // 底色层
    Environment   = 10,   // 绿地 / 水体层
    RoadBase      = 20,   // 道路底面层（宽灰线）
    RoadTop       = 30,   // 道路顶面层（窄白线）
    Building      = 40,   // 建筑轮廓层
    RouteHighlight = 50,  // 导航路线层
    LabelIcon     = 60    // 文字 / 图标层
};

#endif // MAPLAYER_H
