const { searchPois, getBuildings } = require('../../services/poi');

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

Page({
  data: {
    keyword: '',
    results: [],
    searching: false,
    mapLatitude: 34.3849,
    mapLongitude: 108.9863,
    mapScale: 16,
    markers: [
      {
        id: 10001,
        latitude: 34.3849,
        longitude: 108.9863,
        title: '陕西科技大学',
        width: 30,
        height: 30,
        callout: {
          content: '陕西科技大学',
          color: '#1f2d3d',
          bgColor: '#ffffff',
          borderRadius: 8,
          padding: 6,
          display: 'BYCLICK'
        }
      }
    ],
    polyline: []
  },

  onSearchInput(e) {
    const keyword = e.detail.value.trim();
    this.setData({ keyword });
    clearTimeout(searchTimer);
    if (!keyword) {
      this.setData({ results: [], polyline: [] });
      return;
    }
    searchTimer = setTimeout(() => this.doSearch(keyword), 300);
  },

  doSearch(keyword) {
    this.setData({ searching: true });
    searchPois(keyword).then((res) => {
      const { code, data } = res.data;
      this.setData({ searching: false });
      if (code === 0) {
        const pois = data.pois || [];
        if (pois.length) {
          this.setData({ results: pois });
          this.updateMapByPoi(pois[0]);
          return;
        }

        getBuildings().then((buildingRes) => {
          const list = extractBuildingsFromResponse(buildingRes);
          const fallback = fuzzyMatchBuildings(list, keyword);
          this.setData({ results: fallback });
          if (!fallback.length) {
            wx.showToast({ title: '未找到相关建筑，请换个关键词', icon: 'none' });
          }
        }).catch(() => {
          this.setData({ results: [] });
          wx.showToast({ title: '未找到相关建筑，请检查数据源', icon: 'none' });
        });
      }
    }).catch(() => {
      this.setData({ searching: false });
      wx.showToast({ title: '搜索失败，请检查网络设置', icon: 'none' });
    });
  },

  onClearSearch() {
    this.setData({ keyword: '', results: [], polyline: [] });
  },

  onQuickAction(e) {
    const keyword = e.currentTarget.dataset.keyword;
    this.setData({ keyword });
    this.doSearch(keyword);
  },

  onTapPoi(e) {
    const { id } = e.detail;
    const poi = this.data.results.find((item) => item.id === id || String(item.id) === String(id));
    if (poi) {
      this.updateMapByPoi(poi);
      wx.showToast({ title: `已定位到${poi.name}`, icon: 'none' });
    }
  },

  onMarkerTap(e) {
    const markerId = e.detail.markerId;
    const poi = this.data.results.find((item) => Number(item.id) === Number(markerId));
    if (poi) {
      wx.showToast({ title: poi.name, icon: 'none' });
    }
  },

  updateMapByPoi(poi) {
    const lat = Number(poi.latitude) || this.data.mapLatitude + 0.0012;
    const lng = Number(poi.longitude) || this.data.mapLongitude + 0.0015;
    const markers = [this.data.markers[0], {
      id: Number(poi.id) || 90001,
      latitude: lat,
      longitude: lng,
      title: poi.name,
      width: 32,
      height: 32,
      callout: {
        content: poi.name,
        color: '#0f223d',
        bgColor: '#ddebff',
        borderRadius: 8,
        padding: 6,
        display: 'ALWAYS'
      }
    }];

    const polyline = [{
      points: [
        { latitude: this.data.mapLatitude, longitude: this.data.mapLongitude },
        { latitude: lat, longitude: lng }
      ],
      color: '#1677FF',
      width: 6,
      dottedLine: false
    }];

    this.setData({
      markers,
      polyline,
      mapLatitude: lat,
      mapLongitude: lng
    });
  }
});
