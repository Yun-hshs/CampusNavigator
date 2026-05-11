const app = getApp();
const { searchPois } = require('../../services/poi');

let searchTimer = null;

Page({
  data: {
    keyword: '',
    results: [],
    searching: false
  },

  onSearchInput(e) {
    const keyword = e.detail.value.trim();
    this.setData({ keyword });
    clearTimeout(searchTimer);
    if (!keyword) {
      this.setData({ results: [] });
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
        this.setData({ results: data.pois || [] });
      }
    }).catch(() => {
      this.setData({ searching: false });
    });
  },

  onClearSearch() {
    this.setData({ keyword: '', results: [] });
  },

  onQuickAction(e) {
    const keyword = e.currentTarget.dataset.keyword;
    this.setData({ keyword });
    this.doSearch(keyword);
  },

  onTapPoi(e) {
    const { id, name } = e.detail;
    app.globalData.pendingEnd = { id, name };
    wx.switchTab({ url: '/pages/route/route' });
  }
});
