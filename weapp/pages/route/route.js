const app = getApp();
const { searchPois } = require('../../services/poi');
const { planRoute } = require('../../services/route');

let searchTimer = null;

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
        if (code === 0) this.setData({ searchResults: data.pois || [] });
      }).catch(() => this.setData({ searching: false }));
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
