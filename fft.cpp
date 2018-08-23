#include <ffft/FFTReal.h>
#include <iostream>

const int FFTLengthPowerOfTwo = 12; //4096点 ~ 0.1s（44100hz采样频率）
const int SpectrumLengthSamples = 1 << FFTLengthPowerOfTwo;

ffft::FFTReal <float> m_fft(SpectrumLengthSamples);

void calculateSpectrum(const QByteArray &buffer, 
						int byte)
{

}
int main()
{
	/*
	输入len长度得到也是len（偶数）长度，FFT返回结果是对称的，第一个元素为直流量，后面（1，（len-2）/2）,((len-2)/2+2,len-1)
	是实部一样，虚部相反数的复数，中间那个数也是直流量。ffft库考虑到这一点，只输出后半部分，也就是负虚部部分，
	（1，（len-2）/2）为存储实数部分，((len-2)/2+2,len-1)存储虚数部分，中间那个值不变。
	输出处理：求模后乘2除以len得到真实幅度。
	*/
	const long len = 8;
	float x[len] = { 1,2,3,4,5,6,7,8
	};
	float f[len];

	ffft::FFTReal <float> fft_object(len);
	fft_object.do_fft(f, x);
	fft_object.rescale(x);

	for (int i = 0; i < 8; i++)
	{
		if (i == 4)
			std::cout << "--------------" << std::endl;
		std::cout << f[i] << std::endl;
	}
	return 0;

	//QT
	const char *ptr = buffer.constData();
	for (int i = 0; i < m_numSamples; ++i) {
		const qint16 pcmSample = *reinterpret_cast<const qint16*>(ptr);
		// Scale down to range [-1.0, 1.0]
		const DataType realSample = pcmToReal(pcmSample);
		const DataType windowedSample = realSample * m_window[i];
		m_input[i] = windowedSample;
		ptr += bytesPerSample;
	}

	// Calculate the FFT
	m_fft->calculateFFT(m_output.data(), m_input.data());

	qreal min = qreal(2 * inputFrequency) / (m_numSamples);
	qreal max = min;
	// Analyze output to obtain amplitude and phase for each frequency
	for (int i = 2; i <= m_numSamples / 2; ++i) {
		// Calculate frequency of this complex sample
		m_spectrum[i].frequency = qreal(i * inputFrequency) / (m_numSamples);

		//
		min = qMin(min, m_spectrum[i].frequency);
		max = qMax(max, m_spectrum[i].frequency);
		//
		const qreal real = m_output[i];
		qreal imag = 0.0;
		if (i > 0 && i < m_numSamples / 2)
			imag = m_output[m_numSamples / 2 + i];

		const qreal magnitude = qSqrt(real*real + imag * imag);
		qreal amplitude = SpectrumAnalyserMultiplier * qLn(magnitude);

		// Bound amplitude to [0.0, 1.0]
		m_spectrum[i].clipped = (amplitude > 1.0);
		amplitude = qMax(qreal(0.0), amplitude);
		amplitude = qMin(qreal(1.0), amplitude);
		m_spectrum[i].amplitude = amplitude;
	}
	qDebug() << "SpectrumAnalyserThread::calculateSpectrum"
		<< "min" << min
		<< "max" << max;
#endif

	emit calculationComplete(m_spectrum);
}
