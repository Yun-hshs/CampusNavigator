const storage = require('../../services/storage');
const app = getApp();

Page({
  data: {
    list: [],
    isEmpty: true
  },

  onShow() {
    this.refreshList();
  },

  refreshList() {
    const list = storage.getFavorites();
    list.forEach(item => {
      item.addTimeFormatted = storage.formatRelativeTime(item.addTime);
    });
    this.setData({ list, isEmpty: list.length === 0 });
  },

  onTapItem(e) {
    const { buildingId, name, latitude, longitude } = e.currentTarget.dataset;
    app.globalData.pendingEnd = {
      id: Number(buildingId),
      name,
      latitude: Number(latitude),
      longitude: Number(longitude)
    };
    wx.switchTab({ url: '/pages/home/home' });
  },

  onDeleteItem(e) {
    const id = e.currentTarget.dataset.id;
    wx.showModal({
      title: '提示',
      content: '确定取消收藏该地点？',
      confirmColor: '#667eea',
      success: (res) => {
        if (res.confirm) {
          storage.removeFavorite(id);
          this.refreshList();
          wx.showToast({ title: '已取消收藏', icon: 'none' });
        }
      }
    });
  },

  onClearAll() {
    if (this.data.isEmpty) return;
    wx.showModal({
      title: '提示',
      content: '确定清空所有收藏？',
      confirmColor: '#667eea',
      success: (res) => {
        if (res.confirm) {
          wx.removeStorageSync('favorites');
          this.refreshList();
          wx.showToast({ title: '已清空', icon: 'none' });
        }
      }
    });
  }
});
