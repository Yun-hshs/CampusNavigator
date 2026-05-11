const app = getApp();
const { searchPois } = require('../../services/poi');

let searchTimer = null;

Page({
  data: {
    keyword: '',
    results: [],
    searching: false,
    mapLatitude: 31.2304,
    mapLongitude: 121.4737,
    mapScale: 16,
    markers: [
      {
        id: 10001,
        latitude: 31.2304,
        longitude: 121.4737,
        title: '校园中心',
        width: 30,
        height: 30,
        callout: {
          content: '校园中心',
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
        this.setData({ results: pois });
        if (pois.length) {
          this.updateMapByPoi(pois[0]);
        }
      }
    }).catch(() => {
      this.setData({ searching: false });
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
    const { id, name } = e.detail;
    const poi = this.data.results.find((item) => item.id === id || String(item.id) === String(id));
    if (poi) {
      this.updateMapByPoi(poi);
    }
    app.globalData.pendingEnd = { id, name };
    wx.switchTab({ url: '/pages/route/route' });
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
