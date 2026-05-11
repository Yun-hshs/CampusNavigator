const { request } = require('./request');

function searchPois(keyword) {
  return request({ url: '/pois', data: { keyword } });
}

function getPoiDetail(id) {
  return request({ url: `/poi/${id}` });
}

function getBuildings() {
  return request({ url: '/buildings' });
}

module.exports = { searchPois, getPoiDetail, getBuildings };
