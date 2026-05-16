const storage = require('../../services/storage');

Page({
  data: {
    content: '',
    contact: '',
    feedbackList: [],
    submitting: false
  },

  onShow() {
    this.refreshList();
  },

  refreshList() {
    const feedbackList = storage.getFeedbacks();
    feedbackList.forEach(item => {
      item.timeFormatted = storage.formatRelativeTime(item.timestamp);
    });
    this.setData({ feedbackList });
  },

  onContentInput(e) {
    this.setData({ content: e.detail.value });
  },

  onContactInput(e) {
    this.setData({ contact: e.detail.value });
  },

  onSubmit() {
    const content = this.data.content.trim();
    if (!content) {
      wx.showToast({ title: '请输入反馈内容', icon: 'none' });
      return;
    }
    if (content.length > 500) {
      wx.showToast({ title: '内容不能超过500字', icon: 'none' });
      return;
    }
    this.setData({ submitting: true });
    storage.addFeedback({
      content,
      contact: this.data.contact.trim()
    });
    this.setData({ content: '', contact: '', submitting: false });
    this.refreshList();
    wx.showToast({ title: '提交成功', icon: 'success' });
  },

  onDeleteFeedback(e) {
    const id = e.currentTarget.dataset.id;
    wx.showModal({
      title: '提示',
      content: '确定删除该条反馈？',
      confirmColor: '#667eea',
      success: (res) => {
        if (res.confirm) {
          storage.removeFeedback(id);
          this.refreshList();
          wx.showToast({ title: '已删除', icon: 'none' });
        }
      }
    });
  }
});
