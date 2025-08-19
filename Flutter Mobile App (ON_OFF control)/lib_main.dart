import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_database/firebase_database.dart';
import 'firebase_options.dart';

const String deviceId = 'lamp01'; // must match ESP8266 DEVICE_ID
final DatabaseReference stateRef =
    FirebaseDatabase.instance.ref('devices/$deviceId/state');
final DatabaseReference statusRef =
    FirebaseDatabase.instance.ref('devices/$deviceId/status');

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp(
    options: DefaultFirebaseOptions.currentPlatform,
  );
  runApp(const LampApp());
}

class LampApp extends StatelessWidget {
  const LampApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Lamp Controller',
      theme: ThemeData(
        colorSchemeSeed: Colors.teal,
        brightness: Brightness.dark,
        useMaterial3: true,
      ),
      home: const LampPage(),
    );
  }
}

class LampPage extends StatefulWidget {
  const LampPage({super.key});

  @override
  State<LampPage> createState() => _LampPageState();
}

class _LampPageState extends State<LampPage> {
  bool _isOn = false;

  @override
  void initState() {
    super.initState();
    // Listen current state (from DB)
    stateRef.onValue.listen((event) {
      final val = event.snapshot.value?.toString().toUpperCase() ?? 'OFF';
      final newState = (val == 'ON');
      if (mounted && newState != _isOn) {
        setState(() => _isOn = newState);
      }
    });
  }

  Future<void> _toggle(bool value) async {
    setState(() => _isOn = value);
    await stateRef.set(value ? 'ON' : 'OFF');
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Lamp Controller'),
      ),
      body: Center(
        child: ConstrainedBox(
          constraints: const BoxConstraints(maxWidth: 420),
          child: Card(
            margin: const EdgeInsets.all(24),
            child: Padding(
              padding: const EdgeInsets.all(24),
              child: Column(
                mainAxisSize: MainAxisSize.min,
                children: [
                  const Icon(Icons.lightbulb, size: 96),
                  const SizedBox(height: 16),
                  Text(
                    _isOn ? 'Lamp is ON' : 'Lamp is OFF',
                    style: const TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
                  ),
                  const SizedBox(height: 16),
                  Switch.adaptive(
                    value: _isOn,
                    onChanged: _toggle,
                  ),
                  const SizedBox(height: 24),
                  StreamBuilder(
                    stream: statusRef.onValue,
                    builder: (context, snapshot) {
                      final status = snapshot.hasData
                          ? (snapshot.data! as DatabaseEvent)
                                  .snapshot
                                  .value
                                  ?.toString()
                                  .toUpperCase() ??
                              'UNKNOWN'
                          : 'UNKNOWN';
                      return Text('Device status: $status');
                    },
                  ),
                ],
              ),
            ),
          ),
        ),
      ),
    );
  }
}
