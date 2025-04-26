
---@class Vector
---@field x number
---@field y number
---@field z number
local Vector = {}
function Vector:new(x, y, z) end

---@class Rotator
---@field Pitch number
---@field Yaw number
---@field Roll number
local Rotator = {}
function Rotator:new(p, y, r) end

---@class FString
local FString = {}
---@return string
function FString:ToString() end

---@class GameObject
---@field Location Vector
---@field Rotation Rotator
---@field Velocity Vector
---@field UUID string
local GameObject = {}

function GameObject:PrintLocation() end

---@class Input
local Input = {}
---@param key EKeys
---@return boolean
function Input:GetKey(key) end
---@param key EKeys
---@return boolean
function Input:GetKeyDown(key) end
---@param key EKeys
---@return boolean
function Input:GetKeyUp(key) end

---@enum EKeys
local EKeys = {
    A=0, B=1, C=2, D=3, E=4, F=5, G=6, H=7, I=8, J=9,
    K=10, L=11, M=12, N=13, O=14, P=15, Q=16, R=17,
    S=18, T=19, U=20, V=21, W=22, X=23, Y=24, Z=25,
    SpaceBar=26,
    LeftMouseButton=27,
    RightMouseButton=28,
}
