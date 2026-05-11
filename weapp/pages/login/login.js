const app = getApp();
const { request } = require('../../services/request');

Page({
  onShow() {
    if (app.globalData.token) {
      wx.switchTab({ url: '/pages/home/home' });
    }
  },

  onWechatLogin() {
    wx.showLoading({ title: '登录中…' });
    wx.login({
      success: ({ code }) => {
        if (!code) {
          wx.hideLoading();
          wx.showToast({ title: '获取登录凭证失败', icon: 'none' });
          return;
        }
        request({
          url: '/auth/wx-login',
          method: 'POST',
          data: { code }
        }).then((res) => {
          wx.hideLoading();
          const { code: errCode, data, message } = res.data;
          if (errCode !== 0) {
            wx.showToast({ title: message || '登录失败', icon: 'none' });
            return;
          }
          app.globalData.token = data.token;
          app.globalData.userInfo = data.userInfo || null;
          wx.setStorageSync('token', data.token);
          wx.setStorageSync('userInfo', data.userInfo || null);
          wx.switchTab({ url: '/pages/home/home' });
        }).catch(() => {
          wx.hideLoading();
          wx.showToast({ title: '网络错误，请重试', icon: 'none' });
        });
      },
      fail() {
        wx.hideLoading();
        wx.showToast({ title: '微信登录调用失败', icon: 'none' });
      }
    });
  },
  onGuestMode() {
    wx.switchTab({ url: '/pages/home/home' });
  }
});
