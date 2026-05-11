const app = getApp();

function request({ url, method = 'GET', data = {} }) {
  return new Promise((resolve, reject) => {
    wx.request({
      url: `${app.globalData.baseURL}${url}`,
      method,
      data,
      header: {
        Authorization: `Bearer ${app.globalData.token || ''}`
      },
      success: resolve,
      fail: reject
    });
  });
}

module.exports = { request };
