App({
  globalData: {
    token: '',
    userInfo: null,
    baseURL: 'http://127.0.0.1:8080/api'
  },

  onLaunch() {
    const token = wx.getStorageSync('token');
    const userInfo = wx.getStorageSync('userInfo');
    if (token) {
      this.globalData.token = token;
      this.globalData.userInfo = userInfo || null;
    }
  }
});
