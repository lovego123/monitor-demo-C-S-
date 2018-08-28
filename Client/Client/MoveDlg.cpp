#include "stdafx.h"
#include "MoveDlg.h"


MoveDlg::MoveDlg()
{
}


MoveDlg::~MoveDlg()
{
}
void MoveDlg::OnSize(const CDialog * dlg, CRect &rect, UINT nType, int cx, int cy)
{
	if (nType != SIZE_MINIMIZED) //�жϴ����ǲ�����С���ˣ���Ϊ������С��֮�� �����ڵĳ��Ϳ����0����ǰһ�α仯��ʱ�ͻ���ֳ���0�Ĵ������
	{
		CWnd* pChildWnd = dlg->GetWindow(GW_CHILD);
		UINT nCtrlID = 0;
		while (pChildWnd != NULL) //������ÿ���ؼ����ı�
		{
			nCtrlID = pChildWnd->GetDlgCtrlID();
			ChangeSize(dlg, rect, nCtrlID, cx, cy);
			pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
		}

		dlg->GetClientRect(&rect); //���Ҫ���¶Ի���Ĵ�С��������һ�α仯�ľ�����
	}
}

void MoveDlg::ChangeSize(const CDialog *dlg, CRect &m_rect, UINT nID, int x, int y)//nIDΪ�ؼ�ID��x, y�ֱ�Ϊ�Ի���ĵ�ǰ���Ϳ�
{
	CWnd *pWnd = dlg->GetDlgItem(nID);
	if (pWnd != NULL) //�ж��Ƿ�Ϊ�գ���Ϊ�ڴ��ڴ�����ʱ��Ҳ�����OnSize���������Ǵ�ʱ�����ؼ���û�д�����PwndΪ��
	{
		CRect rec;
		pWnd->GetWindowRect(&rec); //��ȡ�ؼ��仯ǰ�Ĵ�С
		dlg->ScreenToClient(&rec); //���ؼ���Сװ��λ�ڶԻ����е���������
		rec.left = rec.left*x / m_rect.Width(); //���ձ��������ռ����λ��
		rec.top = rec.top*y / m_rect.Height();
		rec.bottom = rec.bottom*y / m_rect.Height();
		rec.right = rec.right*x / m_rect.Width();
		pWnd->MoveWindow(rec); //�����ؼ�
	}
}