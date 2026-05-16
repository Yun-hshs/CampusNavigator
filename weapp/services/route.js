const { request } = require('./request');

function planRoute({ startId, endId }) {
  return request({
    url: '/route',
    method: 'POST',
    data: { start: startId, end: endId }
  });
}

module.exports = { planRoute };
