#pragma once

template<typename T>
constexpr T& min(T& A, T& B) { return A > B ? B : A; }
template<typename T>
constexpr T&& min(T&& A, T&& B) { return A > B ? B : A; }
template<typename T>
constexpr T min(T A, T B) { return A > B ? B : A; }

template<typename T>
constexpr T& max(T& A, T& B) { return A < B ? B : A; }
template<typename T>
constexpr T&& max(T&& A, T&& B) { return A < B ? B : A; }
template<typename T>
constexpr T max(T A, T B) { return A < B ? B : A; }
