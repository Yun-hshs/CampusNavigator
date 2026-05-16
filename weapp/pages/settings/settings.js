const storage = require('../../services/storage');

Page({
  data: {
    favCount: 0,
    historyCount: 0,
    feedbackCount: 0,
    cacheSize: '0',
    version: '1.0.0'
  },

  onShow() {
    this.refreshStats();
  },

  refreshStats() {
    const usage = storage.getStorageUsage();
    this.setData({
      favCount: usage.favorites,
      historyCount: usage.history,
      feedbackCount: usage.feedbacks,
      cacheSize: usage.totalKB
    });
  },

  onClearFavorites() {
    wx.showModal({
      title: '提示',
      content: '确定清空所有收藏数据？',
      confirmColor: '#667eea',
      success: (res) => {
        if (res.confirm) {
          wx.removeStorageSync('favorites');
          this.refreshStats();
          wx.showToast({ title: '已清空收藏', icon: 'none' });
        }
      }
    });
  },

  onClearHistory() {
    wx.showModal({
      title: '提示',
      content: '确定清空所有历史记录？',
      confirmColor: '#667eea',
      success: (res) => {
        if (res.confirm) {
          storage.clearHistory();
          this.refreshStats();
          wx.showToast({ title: '已清空历史', icon: 'none' });
        }
      }
    });
  },

  onClearFeedbacks() {
    wx.showModal({
      title: '提示',
      content: '确定清空所有反馈数据？',
      confirmColor: '#667eea',
      success: (res) => {
        if (res.confirm) {
          wx.removeStorageSync('feedbacks');
          this.refreshStats();
          wx.showToast({ title: '已清空反馈', icon: 'none' });
        }
      }
    });
  },

  onClearAll() {
    wx.showModal({
      title: '警告',
      content: '将清除所有收藏、历史和反馈数据，此操作不可恢复。',
      confirmColor: '#ff4d4f',
      success: (res) => {
        if (res.confirm) {
          storage.clearAll();
          this.refreshStats();
          wx.showToast({ title: '已清除所有数据', icon: 'none' });
        }
      }
    });
  },

  onAbout() {
    wx.showModal({
      title: '关于',
      content: '校园导航系统 v' + this.data.version + '\n陕西科技大学校园地图导航应用',
      showCancel: false,
      confirmText: '知道了',
      confirmColor: '#667eea'
    });
  }
});
