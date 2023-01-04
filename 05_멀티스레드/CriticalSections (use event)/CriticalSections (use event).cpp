#include <windows.h>
#include <stdio.h>

#define MAXCNT 100000000
int g_count = 0;

HANDLE addHandle;
HANDLE subHandle;

DWORD WINAPI MyThread1(LPVOID arg)
{
	DWORD retval;

	for (int i = 0; i < MAXCNT; i++) {
		// 감소 완료 대기
		retval = WaitForSingleObject(subHandle, INFINITE);
		if (retval != WAIT_OBJECT_0) break;

		// 전역 변수에 증가 적용
		g_count += 2;

		// 증가 완료 알림
		SetEvent(addHandle);
	}
	return 0;
}

DWORD WINAPI MyThread2(LPVOID arg)
{
	DWORD retval;

	for (int i = 0; i < MAXCNT; i++) {
		// 증가 완료 대기
		retval = WaitForSingleObject(addHandle, INFINITE);
		if (retval != WAIT_OBJECT_0) break;

		// 전역 변수에 감소 적용
		g_count -= 2;

		// 감소 완료 알림
		SetEvent(subHandle);
	}
	return 0;
}

int main(int argc, char* argv[])
{
	// 두 개의 자동 리셋 이벤트 생성(각각 비신호, 신호 상태)
	addHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (addHandle == NULL) return 1;

	subHandle = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (addHandle == NULL) return 1;

	// 두 개의 스레드 생성
	HANDLE hThread[2];
	hThread[0] = CreateThread(NULL, 0, MyThread1, NULL, 0, NULL);
	hThread[1] = CreateThread(NULL, 0, MyThread2, NULL, 0, NULL);

	// 두 개의 스레드 종료 대기
	WaitForMultipleObjects(2, hThread, TRUE, INFINITE);

	// 결과 출력
	printf("g_count = %d\n", g_count);

	// 이벤트 제거
	CloseHandle(addHandle);
	CloseHandle(subHandle);

	return 0;
}