#ifndef __DATE_H__
#define __DATE_H__

#include<iostream>
#include<iomanip>

class Date
{
private:
	int m_day, m_month, m_year; // dd/mm/yyyy
public:
	static int dayInMonth[12]; // Mảng chứa số ngày trong tháng tương ứng với chỉ số mảng (0 -> 11)
	static bool checkLeapYear(const int &year) { return ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)); } // Phương thức kiểm tra năm nhuận
	Date() { m_day = m_month = m_year = 0; }
	Date(const int &day, const int &month, const int &year)
		: m_day(day), m_month(month), m_year(year) {}
	~Date() { m_day = m_month = m_year = 0; }

	void setDay(const int &day) { m_day = day; }
	void setMonth(const int &month) { m_month = month; }
	void setYear(const int &year) { m_year = year; }
	int getDay() const { return m_day; }
	int getMonth() const { return m_month; }
	int getYear() const { return m_year; }

	friend std::istream& operator>>(std::istream &inDev, Date &obj)
	{
		inDev >> obj.m_day;
		inDev >> obj.m_month;
		inDev >> obj.m_year;
		return inDev;
	}
	friend std::ostream& operator<<(std::ostream &outDev, const Date &obj)
	{
		outDev << std::setw(2) << std::left << obj.m_day << " ";
		outDev << std::setw(2) << std::left << obj.m_month << " ";
		outDev << std::setw(4) << std::left << obj.m_year;
		return outDev;
	}
};
int Date::dayInMonth[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
int operator-(const Date &obj1, const Date &obj2) // Phương thức tính khoảng cách giữa 2 Date (trả ra số ngày)
{
	int d1, d2, m1, m2, y1, y2;
	d1 = obj1.getDay(); d2 = obj2.getDay();
	m1 = obj1.getMonth() - 1; m2 = obj2.getMonth() - 1;
	y1 = obj1.getYear(); y2 = obj2.getYear();
	if (y1 < y2 || y1 == y2 && m1 < m2 || m1 == m2 && d1 < d2)
		return -1;
	if (y1 == y2 && m1 == m2)
		return (d1 - d2);
	int sumOfDay = d1 + Date::dayInMonth[m2] - d2;
	while (y1 != y2)
	{
		for (int i = 0; i < m1; i++)
			sumOfDay += Date::dayInMonth[i];
		if (m1 > 1 && Date::checkLeapYear(y1))
			sumOfDay += 1;
		m1 = 12; y1--;
	}
	m1 = (y2 != obj1.getYear()) ? 12 : obj1.getMonth();
	for (int i = m2 + 1; i < m1; i++)
		sumOfDay += Date::dayInMonth[i];
	if (m2 <= 1 && Date::checkLeapYear(y2))
		sumOfDay += 1;

	return sumOfDay;
}

#endif