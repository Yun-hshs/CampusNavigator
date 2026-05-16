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
    const list = storage.getHistory();
    list.forEach(item => {
      item.timeFormatted = storage.formatRelativeTime(item.timestamp);
      item.modeText = item.mode === 'bike' ? '骑行' : '步行';
    });
    this.setData({ list, isEmpty: list.length === 0 });
  },

  onTapItem(e) {
    const { startId, startName, endId, endName } = e.currentTarget.dataset;
    app.globalData.pendingRoute = {
      startId: Number(startId),
      startName,
      endId: Number(endId),
      endName
    };
    wx.switchTab({ url: '/pages/route/route' });
  },

  onDeleteItem(e) {
    const id = e.currentTarget.dataset.id;
    wx.showModal({
      title: '提示',
      content: '确定删除该条记录？',
      confirmColor: '#667eea',
      success: (res) => {
        if (res.confirm) {
          storage.removeHistory(id);
          this.refreshList();
          wx.showToast({ title: '已删除', icon: 'none' });
        }
      }
    });
  },

  onClearAll() {
    if (this.data.isEmpty) return;
    wx.showModal({
      title: '提示',
      content: '确定清空所有历史记录？',
      confirmColor: '#667eea',
      success: (res) => {
        if (res.confirm) {
          storage.clearHistory();
          this.refreshList();
          wx.showToast({ title: '已清空', icon: 'none' });
        }
      }
    });
  }
});
