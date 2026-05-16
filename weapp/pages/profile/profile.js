Page({
  data: {
    statusBarHeight: 20,
    avatarUrl: '/images/school_badge.png',
    nickname: '校园导航用户',
    studentId: '',
    favCount: 0,
    historyCount: 0,
    feedbackCount: 0
  },

  onLoad() {
    const sysInfo = wx.getSystemInfoSync();
    this.setData({ statusBarHeight: sysInfo.statusBarHeight || 20 });
  },

  onShow() {
    this.loadUserInfo();
    this.loadStats();
  },

  loadUserInfo() {
    const userInfo = wx.getStorageSync('userInfo');
    if (userInfo) {
      this.setData({
        avatarUrl: userInfo.avatarUrl || '/images/school_badge.png',
        nickname: userInfo.nickname || '校园导航用户',
        studentId: userInfo.studentId || ''
      });
    }
  },

  loadStats() {
    const favorites = wx.getStorageSync('favorites') || [];
    const history = wx.getStorageSync('history') || [];
    const feedbacks = wx.getStorageSync('feedbacks') || [];
    this.setData({
      favCount: favorites.length,
      historyCount: history.length,
      feedbackCount: feedbacks.length
    });
  },

  onTapAvatar() {
    wx.getUserProfile({
      desc: '用于展示用户信息',
      success: (res) => {
        const userInfo = res.userInfo;
        wx.setStorageSync('userInfo', userInfo);
        this.setData({
          avatarUrl: userInfo.avatarUrl,
          nickname: userInfo.nickName
        });
      },
      fail: () => {
        wx.showToast({ title: '授权已取消', icon: 'none' });
      }
    });
  },

  goFavorites() {
    wx.navigateTo({ url: '/pages/favorites/favorites' });
  },

  goHistory() {
    wx.navigateTo({ url: '/pages/history/history' });
  },

  goFeedback() {
    wx.navigateTo({ url: '/pages/feedback/feedback' });
  },

  goSettings() {
    wx.navigateTo({ url: '/pages/settings/settings' });
  },

  goAbout() {
    wx.showModal({
      title: '关于',
      content: '校园导航系统 v1.0\n陕西科技大学校园地图导航应用',
      showCancel: false,
      confirmText: '知道了',
      confirmColor: '#667eea'
    });
  }
});
