const app = getApp();
const { searchPois, getBuildings } = require('../../services/poi');
const { planRoute } = require('../../services/route');
const storage = require('../../services/storage');

let searchTimer = null;

function extractBuildingsFromResponse(res) {
  const payload = (res && res.data) || {};
  const data = payload.data || {};
  if (Array.isArray(data.buildings)) return data.buildings;
  if (Array.isArray(data)) return data;
  if (Array.isArray(payload.buildings)) return payload.buildings;
  return [];
}

function fuzzyMatchBuildings(buildings, keyword) {
  const key = String(keyword || '').trim().toLowerCase();
  if (!key) return [];
  return (buildings || []).filter((item) => {
    const name = String(item.name || '').toLowerCase();
    const type = String(item.type || '').toLowerCase();
    const aliases = Array.isArray(item.aliases) ? item.aliases.join(' ').toLowerCase() : String(item.aliases || '').toLowerCase();
    return name.includes(key) || type.includes(key) || aliases.includes(key);
  });
}

function bearingToDirection(bearing) {
  const dirs = ['北', '东北', '东', '东南', '南', '西南', '西', '西北'];
  const idx = Math.round(bearing / 45) % 8;
  return dirs[idx];
}

function haversineDistance(lat1, lng1, lat2, lng2) {
  const R = 6371000;
  const toRad = d => d * Math.PI / 180;
  const dLat = toRad(lat2 - lat1);
  const dLng = toRad(lng2 - lng1);
  const a = Math.sin(dLat / 2) ** 2 +
    Math.cos(toRad(lat1)) * Math.cos(toRad(lat2)) * Math.sin(dLng / 2) ** 2;
  return R * 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
}

function computeBearing(lat1, lng1, lat2, lng2) {
  const toRad = d => d * Math.PI / 180;
  const toDeg = r => r * 180 / Math.PI;
  const dLng = toRad(lng2 - lng1);
  const y = Math.sin(dLng) * Math.cos(toRad(lat2));
  const x = Math.cos(toRad(lat1)) * Math.sin(toRad(lat2)) -
    Math.sin(toRad(lat1)) * Math.cos(toRad(lat2)) * Math.cos(dLng);
  return (toDeg(Math.atan2(y, x)) + 360) % 360;
}

function generateSteps(path, totalDistance) {
  if (!path || path.length < 2) return [];
  const steps = [];
  for (let i = 0; i < path.length - 1; i++) {
    const from = path[i];
    const to = path[i + 1];
    const bearing = computeBearing(from.latitude, from.longitude, to.latitude, to.longitude);
    const direction = bearingToDirection(bearing);
    const segDist = Math.round(haversineDistance(from.latitude, from.longitude, to.latitude, to.longitude));
    const isLast = i === path.length - 2;
    steps.push({
      index: i + 1,
      fromName: from.name,
      toName: to.name,
      direction,
      distance: segDist || 10,
      isLast
    });
  }
  return steps;
}

function buildMarkers(path) {
  if (!path || path.length < 2) return [];
  const markers = [];
  // start marker (green)
  markers.push({
    id: 10001,
    latitude: path[0].latitude,
    longitude: path[0].longitude,
    title: '起点: ' + path[0].name,
    width: 30,
    height: 30,
    callout: {
      content: '起点: ' + path[0].name,
      color: '#ffffff',
      bgColor: '#52c41a',
      borderRadius: 10,
      padding: 10,
      display: 'ALWAYS',
      textAlign: 'center'
    }
  });
  // end marker (red)
  const last = path[path.length - 1];
  markers.push({
    id: 10002,
    latitude: last.latitude,
    longitude: last.longitude,
    title: '终点: ' + last.name,
    width: 30,
    height: 30,
    callout: {
      content: '终点: ' + last.name,
      color: '#ffffff',
      bgColor: '#f5222d',
      borderRadius: 10,
      padding: 10,
      display: 'ALWAYS',
      textAlign: 'center'
    }
  });
  // waypoint markers (blue, only if > 2 points)
  if (path.length > 2) {
    for (let i = 1; i < path.length - 1; i++) {
      markers.push({
        id: 10003 + i,
        latitude: path[i].latitude,
        longitude: path[i].longitude,
        title: path[i].name,
        width: 20,
        height: 20,
        callout: {
          content: path[i].name,
          color: '#333333',
          bgColor: '#ffffff',
          borderRadius: 8,
          padding: 8,
          display: 'BYCLICK',
          textAlign: 'center'
        }
      });
    }
  }
  return markers;
}

function formatDistance(meters) {
  if (meters >= 1000) return (meters / 1000).toFixed(1) + 'km';
  return Math.round(meters) + 'm';
}

function formatTime(minutes) {
  if (minutes < 1) return '不到1分钟';
  if (minutes >= 60) {
    const h = Math.floor(minutes / 60);
    const m = Math.round(minutes % 60);
    return m > 0 ? h + '小时' + m + '分钟' : h + '小时';
  }
  return '约' + Math.ceil(minutes) + '分钟';
}

Page({
  data: {
    startId: null,
    startName: '',
    endId: null,
    endName: '',
    mode: 'walk',
    modes: [
      { value: 'walk', label: '步行', icon: '🚶' },
      { value: 'bike', label: '骑行', icon: '🚲' }
    ],
    // map
    mapLatitude: 34.3819,
    mapLongitude: 108.9839,
    mapScale: 16,
    markers: [],
    polyline: [],
    // search modal
    showSearch: false,
    searchTarget: '',
    searchKeyword: '',
    searchResults: [],
    searching: false,
    // route result
    loading: false,
    route: null,
    steps: [],
    displayDistance: '',
    displayTime: '',
    error: '',
    showResult: false
  },

  onReady() {
    this.mapCtx = wx.createMapContext('routeMap', this);
  },

  onShow() {
    const pending = app.globalData.pendingEnd;
    if (pending) {
      this.setData({ endId: pending.id, endName: pending.name });
      app.globalData.pendingEnd = null;
    }
    const pendingRoute = app.globalData.pendingRoute;
    if (pendingRoute) {
      this.setData({
        startId: pendingRoute.startId,
        startName: pendingRoute.startName,
        endId: pendingRoute.endId,
        endName: pendingRoute.endName
      });
      app.globalData.pendingRoute = null;
      this.planRoute();
    }
  },

  // --- mode ---
  onModeChange(e) {
    const newMode = e.currentTarget.dataset.mode;
    if (newMode === this.data.mode) return;
    this.setData({ mode: newMode });
    if (this.data.route) {
      this.updateDisplayForMode();
    }
  },

  // --- swap ---
  onSwap() {
    const { startId, startName, endId, endName } = this.data;
    this.setData({
      startId: endId,
      startName: endName,
      endId: startId,
      endName: startName
    });
    if (this.data.route) {
      this.planRoute();
    }
  },

  // --- open search picker ---
  openSearch(e) {
    this.setData({
      showSearch: true,
      searchTarget: e.currentTarget.dataset.target,
      searchKeyword: '',
      searchResults: []
    });
  },

  onCloseSearch() {
    this.setData({ showSearch: false });
  },

  onClearSearch() {
    this.setData({ searchKeyword: '', searchResults: [] });
  },

  onSearchInput(e) {
    const keyword = (e.detail.value || '').trim();
    this.setData({ searchKeyword: keyword });
    clearTimeout(searchTimer);
    if (!keyword) {
      this.setData({ searchResults: [] });
      return;
    }
    searchTimer = setTimeout(() => {
      this.setData({ searching: true });
      searchPois(keyword).then((res) => {
        const { code, data } = res.data;
        this.setData({ searching: false });
        if (code === 0) {
          const pois = data.pois || [];
          if (pois.length) {
            this.setData({ searchResults: pois });
            return;
          }
          getBuildings().then((buildingRes) => {
            const list = extractBuildingsFromResponse(buildingRes);
            const fallback = fuzzyMatchBuildings(list, keyword);
            this.setData({ searchResults: fallback });
            if (!fallback.length) {
              wx.showToast({ title: '未找到相关建筑，请换个关键词', icon: 'none' });
            }
          }).catch(() => {
            this.setData({ searchResults: [] });
            wx.showToast({ title: '未找到相关建筑，请检查数据源', icon: 'none' });
          });
        }
      }).catch(() => {
        this.setData({ searching: false });
        wx.showToast({ title: '搜索失败，请检查网络设置', icon: 'none' });
      });
    }, 300);
  },

  onSelectPoi(e) {
    const { id, name, latitude, longitude } = e.detail;
    const target = this.data.searchTarget;
    if (target === 'start') {
      this.setData({ startId: id, startName: name });
    } else {
      this.setData({ endId: id, endName: name });
    }
    this.setData({ showSearch: false });
    if (latitude && longitude) {
      const lat = Number(latitude);
      const lng = Number(longitude);
      this.setData({ mapLatitude: lat, mapLongitude: lng, mapScale: 17 });
    }
  },

  // --- plan route ---
  onPlanRoute() {
    this.planRoute();
  },

  planRoute() {
    const { startId, endId } = this.data;
    if (!startId || !endId) {
      wx.showToast({ title: '请选择起点和终点', icon: 'none' });
      return;
    }
    if (startId === endId) {
      wx.showToast({ title: '起点和终点不能相同', icon: 'none' });
      return;
    }
    this.setData({ loading: true, route: null, steps: [], error: '', showResult: false });
    planRoute({ startId, endId }).then((res) => {
      const { code, data, message } = res.data;
      this.setData({ loading: false });
      if (code === 0 && data && data.path) {
        const path = data.path;
        const walkTime = data.time || 0;
        const distance = data.distance || 0;

        // Build map elements
        const polyline = [{
          points: path.map(p => ({ latitude: p.latitude, longitude: p.longitude })),
          color: '#667eea',
          width: 6,
          dottedLine: false,
          arrowLine: true
        }];
        const markers = buildMarkers(path);
        const steps = generateSteps(path, distance);

        // Compute center of path for map view
        const centerLat = path.reduce((s, p) => s + p.latitude, 0) / path.length;
        const centerLng = path.reduce((s, p) => s + p.longitude, 0) / path.length;

        this.setData({
          route: { distance, walkTime, path },
          steps,
          markers,
          polyline,
          mapLatitude: centerLat,
          mapLongitude: centerLng,
          mapScale: 16,
          showResult: true
        });

        this.updateDisplayForMode();

        // Fit map to show full route
        if (this.mapCtx) {
          this.mapCtx.includePoints({
            points: path.map(p => ({ latitude: p.latitude, longitude: p.longitude })),
            padding: [60, 60, 60, 60]
          });
        }

        // Save to history
        storage.addHistory({
          startId: String(this.data.startId),
          startName: this.data.startName,
          endId: String(this.data.endId),
          endName: this.data.endName,
          distance,
          time: Math.ceil(walkTime),
          mode: this.data.mode
        });
      } else {
        this.setData({ error: message || '路线规划失败' });
      }
    }).catch(() => {
      this.setData({ loading: false, error: '网络错误，请重试' });
    });
  },

  updateDisplayForMode() {
    const { route, mode } = this.data;
    if (!route) return;
    const time = mode === 'bike' ? route.walkTime / 3 : route.walkTime;
    this.setData({
      displayDistance: formatDistance(route.distance),
      displayTime: formatTime(time)
    });
  },

  onCloseResult() {
    this.setData({ showResult: false });
  },

  onToggleResult() {
    this.setData({ showResult: !this.data.showResult });
  },

  // clear route and reset
  onClearRoute() {
    this.setData({
      startId: null,
      startName: '',
      endId: null,
      endName: '',
      route: null,
      steps: [],
      markers: [],
      polyline: [],
      error: '',
      showResult: false,
      displayDistance: '',
      displayTime: ''
    });
  }
});
