const { request } = require('../../services/request');

let searchTimer = null;

Page({
  data: {
    keyword: '',
    results: [],
    searching: false,
    mapLatitude: 34.3819,
    mapLongitude: 108.9839,
    mapScale: 16,
    markers: [],
    polyline: [],
    buildingCount: 0,
    showRoute: false,
    routeStart: '',
    routeEnd: '',
    routeDistance: 0,
    routeTime: 0,
    startPoint: null,
    endPoint: null
  },

  onLoad() {
    this.mapCtx = null;
  },

  onReady() {
    // 页面渲染完成后初始化地图上下文
    this.mapCtx = wx.createMapContext('campusMap', this);
    // 加载建筑数据
    this.loadBuildings();
  },

  onShow() {
    // 每次显示页面时刷新建筑数据
    if (this.mapCtx) {
      this.loadBuildings();
    }
  },

  // 从Qt后端加载建筑数据
  loadBuildings() {
    request({ url: '/buildings' }).then((res) => {
      const { code, data } = res.data;
      if (code === 0) {
        const buildings = data.buildings || [];
        this.setData({ buildingCount: buildings.length });
        this.renderBuildingsOnMap(buildings);
      }
    }).catch((err) => {
      console.error('加载建筑数据失败:', err);
      wx.showToast({ title: '无法连接服务器', icon: 'none' });
    });
  },

  // 在地图上渲染建筑标记
  renderBuildingsOnMap(buildings) {
    const markers = buildings.map((building, index) => ({
      id: Number(building.id) || index + 10000,
      latitude: Number(building.latitude),
      longitude: Number(building.longitude),
      title: building.name,
      width: 30,
      height: 30,
      iconPath: '/images/marker.png', // 使用自定义标记图标
      callout: {
        content: building.name,
        color: '#333333',
        bgColor: '#ffffff',
        borderRadius: 10,
        padding: 10,
        display: 'BYCLICK',
        textAlign: 'center'
      }
    }));

    this.setData({ markers });
  },

  // 搜索输入
  onSearchInput(e) {
    const keyword = e.detail.value.trim();
    this.setData({ keyword });

    clearTimeout(searchTimer);

    if (!keyword) {
      this.setData({ results: [] });
      return;
    }

    // 300ms防抖
    searchTimer = setTimeout(() => this.doSearch(keyword), 300);
  },

  // 搜索确认
  onSearchConfirm() {
    if (this.data.keyword) {
      this.doSearch(this.data.keyword);
    }
  },

  // 执行搜索 - 调用Qt后端API
  doSearch(keyword) {
    this.setData({ searching: true });

    request({
      url: '/pois',
      data: { keyword }
    }).then((res) => {
      const { code, data } = res.data;
      this.setData({ searching: false });

      if (code === 0) {
        const pois = data.pois || [];
        this.setData({ results: pois });

        if (pois.length > 0) {
          // 定位到第一个搜索结果
          this.locateToPoi(pois[0]);
        }
      }
    }).catch(() => {
      this.setData({ searching: false });
      wx.showToast({ title: '搜索失败，请检查网络', icon: 'none' });
    });
  },

  // 清除搜索
  onClearSearch() {
    this.setData({
      keyword: '',
      results: [],
      showRoute: false
    });
    this.loadBuildings(); // 重新加载所有建筑
  },

  // 点击搜索结果
  onTapResult(e) {
    const { id, lat, lng, name } = e.currentTarget.dataset;
    const poi = this.data.results.find(item => String(item.id) === String(id));

    if (poi) {
      this.locateToPoi(poi);
      this.setData({
        keyword: name,
        results: []
      });
      wx.showToast({ title: `已定位到${name}`, icon: 'none' });
    }
  },

  // 定位到POI
  locateToPoi(poi) {
    const lat = Number(poi.latitude);
    const lng = Number(poi.longitude);

    // 添加选中标记
    const selectedMarker = {
      id: 99999,
      latitude: lat,
      longitude: lng,
      title: poi.name,
      width: 40,
      height: 40,
      callout: {
        content: poi.name,
        color: '#ffffff',
        bgColor: '#667eea',
        borderRadius: 10,
        padding: 10,
        display: 'ALWAYS',
        textAlign: 'center'
      }
    };

    // 保留原有标记，添加选中标记
    const markers = [...this.data.markers.filter(m => m.id !== 99999), selectedMarker];

    this.setData({
      markers,
      mapLatitude: lat,
      mapLongitude: lng,
      mapScale: 18
    });
  },

  // 点击地图标记
  onMarkerTap(e) {
    const markerId = e.detail.markerId;
    const marker = this.data.markers.find(m => Number(m.id) === Number(markerId));

    if (marker) {
      wx.showToast({ title: marker.title, icon: 'none' });
    }
  },

  // 地图点击事件
  onMapTap(e) {
    console.log('地图点击:', e);
  },

  // 地图移动事件
  onMapMove(e) {
    // 可以在这里处理地图移动逻辑
  },

  // 快捷搜索
  onQuickSearch(e) {
    const keyword = e.currentTarget.dataset.keyword;
    this.setData({ keyword });
    this.doSearch(keyword);
  },

  // 关闭路线信息
  onCloseRoute() {
    this.setData({
      showRoute: false,
      polyline: []
    });
  },

  // 规划路径 - 调用Qt后端API
  planRoute(startId, endId) {
    request({
      url: '/route',
      method: 'POST',
      data: {
        start: startId,
        end: endId
      }
    }).then((res) => {
      const { code, data } = res.data;
      if (code === 0) {
        const { path, distance, time } = data;

        // 绘制路径
        const polyline = [{
          points: path.map(p => ({
            latitude: p.latitude,
            longitude: p.longitude
          })),
          color: '#667eea',
          width: 6,
          dottedLine: false,
          arrowLine: true
        }];

        this.setData({
          polyline,
          showRoute: true,
          routeStart: this.data.startPoint?.name || '起点',
          routeEnd: this.data.endPoint?.name || '终点',
          routeDistance: Math.round(distance),
          routeTime: Math.ceil(time)
        });
      }
    }).catch(() => {
      wx.showToast({ title: '路径规划失败', icon: 'none' });
    });
  }
});
