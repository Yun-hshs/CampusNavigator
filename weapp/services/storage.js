const KEYS = {
  favorites: 'favorites',
  history: 'history',
  feedbacks: 'feedbacks'
};

function genId() {
  return Date.now().toString(36) + Math.random().toString(36).slice(2, 6);
}

function formatRelativeTime(iso) {
  if (!iso) return '';
  const now = Date.now();
  const t = new Date(iso).getTime();
  const diff = now - t;
  if (diff < 60000) return '刚刚';
  if (diff < 3600000) return Math.floor(diff / 60000) + '分钟前';
  if (diff < 86400000) return Math.floor(diff / 3600000) + '小时前';
  const d = new Date(iso);
  const pad = n => String(n).padStart(2, '0');
  if (diff < 172800000) return '昨天 ' + pad(d.getHours()) + ':' + pad(d.getMinutes());
  return pad(d.getMonth() + 1) + '-' + pad(d.getDate()) + ' ' + pad(d.getHours()) + ':' + pad(d.getMinutes());
}

// ---- Favorites ----

function getFavorites() {
  return wx.getStorageSync(KEYS.favorites) || [];
}

function addFavorite(building) {
  const list = getFavorites();
  const bid = String(building.buildingId);
  if (list.some(f => String(f.buildingId) === bid)) return false;
  list.unshift({
    id: genId(),
    buildingId: bid,
    name: building.name || '',
    type: building.type || '',
    description: building.description || '',
    latitude: building.latitude || 0,
    longitude: building.longitude || 0,
    addTime: new Date().toISOString()
  });
  wx.setStorageSync(KEYS.favorites, list);
  return true;
}

function removeFavorite(id) {
  const list = getFavorites().filter(f => f.id !== id);
  wx.setStorageSync(KEYS.favorites, list);
}

function removeFavoriteByBuildingId(buildingId) {
  const bid = String(buildingId);
  const list = getFavorites().filter(f => String(f.buildingId) !== bid);
  wx.setStorageSync(KEYS.favorites, list);
}

function isFavorite(buildingId) {
  return getFavorites().some(f => String(f.buildingId) === String(buildingId));
}

// ---- History ----

function getHistory() {
  return wx.getStorageSync(KEYS.history) || [];
}

function addHistory(record) {
  const list = getHistory();
  list.unshift({
    id: genId(),
    startId: String(record.startId || ''),
    startName: record.startName || '',
    endId: String(record.endId || ''),
    endName: record.endName || '',
    distance: record.distance || 0,
    time: record.time || 0,
    mode: record.mode || 'walk',
    timestamp: new Date().toISOString()
  });
  if (list.length > 50) list.length = 50;
  wx.setStorageSync(KEYS.history, list);
}

function removeHistory(id) {
  const list = getHistory().filter(h => h.id !== id);
  wx.setStorageSync(KEYS.history, list);
}

function clearHistory() {
  wx.removeStorageSync(KEYS.history);
}

// ---- Feedbacks ----

function getFeedbacks() {
  return wx.getStorageSync(KEYS.feedbacks) || [];
}

function addFeedback(item) {
  const list = getFeedbacks();
  list.unshift({
    id: genId(),
    content: item.content || '',
    contact: item.contact || '',
    timestamp: new Date().toISOString()
  });
  wx.setStorageSync(KEYS.feedbacks, list);
}

function removeFeedback(id) {
  const list = getFeedbacks().filter(f => f.id !== id);
  wx.setStorageSync(KEYS.feedbacks, list);
}

// ---- Utility ----

function clearAll() {
  wx.removeStorageSync(KEYS.favorites);
  wx.removeStorageSync(KEYS.history);
  wx.removeStorageSync(KEYS.feedbacks);
}

function getStorageUsage() {
  const fav = getFavorites();
  const hist = getHistory();
  const fb = getFeedbacks();
  const total = JSON.stringify(fav).length + JSON.stringify(hist).length + JSON.stringify(fb).length;
  return {
    favorites: fav.length,
    history: hist.length,
    feedbacks: fb.length,
    totalKB: (total / 1024).toFixed(1)
  };
}

module.exports = {
  formatRelativeTime,
  getFavorites, addFavorite, removeFavorite, removeFavoriteByBuildingId, isFavorite,
  getHistory, addHistory, removeHistory, clearHistory,
  getFeedbacks, addFeedback, removeFeedback,
  clearAll, getStorageUsage
};
