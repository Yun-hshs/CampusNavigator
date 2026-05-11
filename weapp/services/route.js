const { request } = require('./request');

function planRoute({ startId, endId, mode = 'walk' }) {
  return request({
    url: '/route/plan',
    method: 'POST',
    data: { startId, endId, mode }
  });
}

module.exports = { planRoute };
