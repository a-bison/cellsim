template<typename T>
T cycleEnum(const T& e) {
	return static_cast<T>((e + 1) % ENUM_COUNT<T>);
}

template<typename T>
void CyclableEnum<T>::cycle() {
	e = cycleEnum(e);
}

template<typename T>
CyclableEnum<T>::operator T&() {
	return e;
}
