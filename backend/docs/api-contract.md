# CampusNavigator API Contract (Qt Backend)

## Auth
- POST `/api/auth/wx-login`
- POST `/api/auth/bind-phone`

## POI & Map
- GET `/api/pois?keyword=`
- GET `/api/poi/{id}`
- GET `/api/buildings`

## Route
- POST `/api/route/plan`

### Unified Response
```json
{
  "code": 0,
  "message": "ok",
  "data": {}
}
```
