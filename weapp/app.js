App({
  globalData: {
    token: '',
    userInfo: null,
    // 开发机 localhost 仅模拟器可用；真机调试请改为同一局域网下后端机器 IP，如 http://192.168.1.10:8080/api
    baseURL: 'http://127.0.0.1:8080/api',
    pendingEnd: null,
    pendingRoute: null
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
