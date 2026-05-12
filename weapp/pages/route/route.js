const app = getApp();
const { searchPois, getBuildings } = require('../../services/poi');
const { planRoute } = require('../../services/route');

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
    startId: null,
    startName: '',
    endId: null,
    endName: '',
    mode: 'walk',
    modes: [
      { value: 'walk', label: '步行', icon: '🚶' },
      { value: 'bike', label: '骑行', icon: '🚲' }
    ],
    // search modal
    showSearch: false,
    searchTarget: '',
    searchKeyword: '',
    searchResults: [],
    searching: false,
    // route result
    loading: false,
    route: null,
    error: ''
  },

  onShow() {
    const pending = app.globalData.pendingEnd;
    if (pending) {
      this.setData({ endId: pending.id, endName: pending.name });
      app.globalData.pendingEnd = null;
    }
  },

  // --- mode ---
  onModeChange(e) {
    this.setData({ mode: e.currentTarget.dataset.mode });
    if (this.data.route) this.planRoute();
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
    const { id, name } = e.detail;
    const target = this.data.searchTarget;
    if (target === 'start') {
      this.setData({ startId: id, startName: name });
    } else {
      this.setData({ endId: id, endName: name });
    }
    this.setData({ showSearch: false });
  },

  // --- plan route ---
  onPlanRoute() {
    this.planRoute();
  },

  planRoute() {
    const { startId, endId, mode } = this.data;
    if (!startId || !endId) {
      wx.showToast({ title: '请选择起点和终点', icon: 'none' });
      return;
    }
    this.setData({ loading: true, route: null, error: '' });
    planRoute({ startId, endId, mode }).then((res) => {
      const { code, data, message } = res.data;
      this.setData({ loading: false });
      if (code === 0) {
        this.setData({ route: data });
      } else {
        this.setData({ error: message || '路线规划失败' });
      }
    }).catch(() => {
      this.setData({ loading: false, error: '网络错误，请重试' });
    });
  }
});
