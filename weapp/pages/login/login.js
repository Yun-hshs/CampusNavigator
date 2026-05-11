Page({
  onWechatLogin() {
    wx.login({
      success: (res) => {
        console.log('wx.login code:', res.code);
      }
    });
  },
  onGuestMode() {
    wx.switchTab({ url: '/pages/home/home' });
  }
});
